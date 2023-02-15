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
#include <boost/regex.hpp>

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

std::string convert_float(const std::string &json_str)
{
  static boost::regex re("\\\"([0-9]+\\.{0,1}[0-9]*)\\\"");
  return  boost::regex_replace(json_str, re, "$1", boost::match_default);
}

std::string convert_bool(const std::string &json_str)
{
  static boost::regex re("\\\"(true|false)\\\"");
  return  boost::regex_replace(json_str, re, "$1");
}

void HttpWorker::processRequest(http::request<http::string_body> const &req)
{
  URI uri;
#ifdef USE_STRINGVIEW_TOSTRING
  uri.parseTarget(req.target().to_string());
#else
  uri.parseTarget(req.target());
#endif
  if (mLogCallback != nullptr)
  {
    std::ostringstream ss;
    ss << mSocket.remote_endpoint().address().to_string() << ' '
       << req.method() << ' '
       << req.target();
    (*mLogCallback)(ss.str());
  }
  if (req.method() == http::verb::post && uri.path() == "/factor")
  {
    pt::ptree request;
    std::stringstream iss;
    iss << req.body();
    try
    {
      pt::read_json(iss, request);
    }
    catch (pt::ptree_error e)
    {
      sendBadResponse(http::status::bad_request, e.what());
      return;
    }
    if (request.find("number") == request.not_found())
    {
      sendBadResponse(http::status::bad_request, "field \"number\" is missing");
      return;
    }
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
    std::vector<bigint> factors = primality::factors(x);
    auto t1 = chrono::high_resolution_clock::now();
    auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
    pt::ptree factors_child;
    for (auto const &f : factors)
    {
      pt::ptree child;
      child.put("", f);
      factors_child.push_back(std::make_pair("", child));
    }
    pt::ptree response;
    response.put("number", x.convert_to<std::string>());
    if (factors.empty())
    {
      response.put("factors", "[factors]");
    }
    else
    {
      response.add_child("factors", factors_child);
    }
    response.put("elapsed_msecs", 1e3 * dt.count());
    response.put("isprime", factors.empty());
    std::ostringstream ss;
    pt::write_json(ss, response, true);
    std::string responseStr = ss.str();
    responseStr = convert_bool(responseStr);
    responseStr = convert_float(responseStr);
    boost::replace_all(responseStr, std::string("\"[factors]\""), "[]");
    sendResponse(responseStr, "application/json");
  }
  else if (req.method() == http::verb::post && uri.path() == "/prime")
  {
    pt::ptree request;
    std::stringstream iss;
    iss << req.body();
    try
    {
      pt::read_json(iss, request);
    }
    catch (pt::ptree_error e)
    {
      sendBadResponse(http::status::bad_request, e.what());
      return;
    }
    if (request.find("number") == request.not_found())
    {
      sendBadResponse(http::status::bad_request, "field \"number\" is missing");
      return;
    }
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
    bool isprime = primality::is_prime(x) != primality::mr_result::composite;
    auto t1 = chrono::high_resolution_clock::now();
    auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
    pt::ptree response;
#ifndef WITH_REGEX_REPLACE
    response.put("elapsed_msecs", "[elapsed_msecs]");
    response.put("isprime", "[isprime]");
#else
    response.put("elapsed_msecs",1e3 * dt.count());
    response.put("isprime", isprime);
#endif
    response.put("number", x.convert_to<std::string>());
    std::ostringstream ss;
    pt::write_json(ss, response, true);
    std::string responseStr = ss.str();
#ifndef WITH_REGEX_REPLACE
    boost::replace_all(responseStr, "\"[elapsed_msecs]\"", std::to_string(1e3 * dt.count()));
    boost::replace_all(responseStr, "\"[isprime]\"", isprime ? "true" : "false");
#else
    responseStr = convert_bool(responseStr);
    responseStr = convert_float(responseStr);
#endif
    sendResponse(responseStr, "application/json");
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
    bool isprime = primality::is_prime(x) != primality::mr_result::composite;
    auto t1 = chrono::high_resolution_clock::now();
    auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
    pt::ptree response;
    response.put<std::string>("elapsed_msecs", "[elapsed_msecs]");
    response.put<std::string>("isprime", "[isprime]");
    response.put<std::string>("number", x.convert_to<std::string>());
    std::ostringstream ss;
    pt::write_json(ss, response, false);
    std::string responseStr = ss.str();
    boost::replace_all<std::string>(responseStr, std::string("\"[elapsed_msecs]\""), std::to_string(1e3 * dt.count()));
    boost::replace_all<std::string>(responseStr, std::string("\"[isprime]\""), isprime ? "true" : "false");
    sendResponse(responseStr, "application/json");
  }
  else
  {
    sendBadResponse(http::status::not_found, "");
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
