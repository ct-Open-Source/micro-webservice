/*
 Copyright © 2023 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG - Redaktion c't

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/multiprecision/gmp.hpp>

#include "server.hpp"
#include "httpworker.hpp"
#include "uri.hpp"
#include "number_theory.hpp"

namespace pt = boost::property_tree;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace chrono = std::chrono;
using tcp = boost::asio::ip::tcp;

typedef boost::multiprecision::mpz_int bigint;

HttpWorker::HttpWorker(
    tcp::acceptor &acceptor,
    log_callback_t *logCallback)
    : mAcceptor(acceptor), mLogCallback(logCallback)
{ /* ... */
}

void HttpWorker::start()
{
  accept();
  checkTimeout();
}

void HttpWorker::accept()
{
  beast::error_code ec;
  mSocket.close(ec);
  mBuffer.consume(mBuffer.size());
  mAcceptor.async_accept(
      mSocket,
      [this](beast::error_code ec)
      {
        if (ec)
        {
          accept();
        }
        else
        {
          mReqTimeout.expires_after(Timeout);
          readRequest();
        }
      });
}

void HttpWorker::readRequest()
{
  mParser.emplace();
  http::async_read(
      mSocket,
      mBuffer,
      *mParser,
      [this](beast::error_code ec, std::size_t)
      {
        if (ec)
        {
          accept();
        }
        else
        {
          processRequest(mParser->get());
        }
      });
}

void HttpWorker::processRequest(http::request<http::string_body> const &req)
{
  URI uri;
  uri.parseTarget(req.target());
  if (mLogCallback != nullptr)
  {
    std::ostringstream ss;
    ss << mSocket.remote_endpoint().address().to_string() << ' '
       << req.method() << ' '
       << req.target();
    (*mLogCallback)(ss.str());
  }
  if (req.method() == http::verb::get && uri.path() == "/factor" && uri.query().find("number") != uri.query().end())
  {
    std::string number_str = uri.query().at("number");
    bigint x{0};
    try
    {
      x.assign(number_str);
    }
    catch (...)
    {
      // ignore
    }
    auto t0 = chrono::high_resolution_clock::now();
    std::vector<bigint> factors = number_theory::prime<bigint>::factors(x);
    auto t1 = chrono::high_resolution_clock::now();
    std::sort(factors.begin(), factors.end());
    pt::ptree factors_child;
    for (auto const &f : factors)
    {
      pt::ptree child;
      child.put<std::string>("", f.convert_to<std::string>());
      factors_child.push_back(std::make_pair("", child));
    }
    pt::ptree response;
    response.put<std::string>("number", x.convert_to<std::string>());
    response.add_child("factors", factors_child);
    auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
    response.put<double>("elapsed_msecs", 1e3 * dt.count());
    std::ostringstream ss;
    pt::write_json(ss, response, false);
    sendResponse(ss.str(), "application/json");
  }
  else if (req.method() == http::verb::post && uri.path() == "/prime")
  {
    pt::ptree request;
    std::stringstream ss;
    ss << req.body();
    try {
      pt::read_json(ss, request);
    }
    catch (pt::ptree_error e) {
      sendBadResponse(http::status::bad_request, e.what());
      return;
    }
    if (request.find("number") != request.not_found()) {
      bigint x{0};
      try
      {
        x.assign(request.get<std::string>("number"));
      }
      catch (...)
      {
        sendBadResponse(http::status::bad_request, "field \"number\" must contain a positive integer number");
        return;
      }
      auto t0 = chrono::high_resolution_clock::now();
      bool isprime = number_theory::prime<bigint>::is_prime(x, 1);
      auto t1 = chrono::high_resolution_clock::now();
      pt::ptree response;
      response.put<std::string>("number", x.convert_to<std::string>());
      response.put<bool>("isprime", isprime);
      auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
      response.put<double>("elapsed_msecs", 1e3 * dt.count());
      std::ostringstream ss;
      pt::write_json(ss, response, false);
      sendResponse(ss.str(), "application/json");
    }
    else
    {
      sendBadResponse(http::status::bad_request, "field \"number\" is missing");
    }
  }
  else if (req.method() == http::verb::get && uri.path() == "/prime" && uri.query().find("number") != uri.query().end())
  {
    std::string number_str = uri.query().at("number");
    bigint x{0};
    try
    {
      x.assign(number_str);
    }
    catch (...)
    {
        sendBadResponse(http::status::bad_request, "field \"number\" must contain a positive integer number");
        return;
    }
    auto t0 = chrono::high_resolution_clock::now();
    bool isprime = number_theory::prime<bigint>::is_prime(x, 5);
    auto t1 = chrono::high_resolution_clock::now();
    pt::ptree response;
    response.put<std::string>("number", x.convert_to<std::string>());
    response.put<bool>("isprime", isprime);
    auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
    response.put<double>("elapsed_msecs", 1e3 * dt.count());
    std::ostringstream ss;
    pt::write_json(ss, response, false);
    sendResponse(ss.str(), "application/json");
  }
  else
  {
    sendBadResponse(http::status::bad_request, "");
  }
}

void HttpWorker::sendResponse(const std::string &body, const std::string &mimetype)
{
  mResponse.emplace();
  mResponse->result(http::status::ok);
  mResponse->set(http::field::server, std::string("Micro server ") + SERVER_VERSION);
  mResponse->set(http::field::content_type, mimetype);
  mResponse->set(http::field::access_control_allow_origin, "*");
  mResponse->body() = body;
  mResponse->prepare_payload();
  mSerializer.emplace(*mResponse);
  http::async_write(
      mSocket,
      *mSerializer,
      [this](boost::beast::error_code ec, std::size_t)
      {
        mSocket.shutdown(tcp::socket::shutdown_send, ec);
        mSerializer.reset();
        mResponse.reset();
        accept();
      });
}

void HttpWorker::sendBadResponse(http::status status, const std::string &error)
{
  mResponse.emplace();
  mResponse->result(status);
  mResponse->set(http::field::server, std::string("Micro server ") + SERVER_VERSION);
  mResponse->set(http::field::content_type, "text/plain");
  mResponse->body() = error;
  mResponse->prepare_payload();
  mSerializer.emplace(*mResponse);
  http::async_write(
      mSocket,
      *mSerializer,
      [this](beast::error_code ec, std::size_t)
      {
        mSocket.shutdown(tcp::socket::shutdown_send, ec);
        mSerializer.reset();
        mResponse.reset();
        accept();
      });
}

void HttpWorker::checkTimeout()
{
  if (mReqTimeout.expiry() <= std::chrono::steady_clock::now())
  {
    mSocket.close();
    mReqTimeout.expires_at(std::chrono::steady_clock::time_point::max());
  }
  mReqTimeout.async_wait(
      [this](beast::error_code)
      {
        checkTimeout();
      });
}
