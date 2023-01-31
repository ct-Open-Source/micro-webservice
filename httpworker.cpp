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

#include "server.hpp"
#include "httpworker.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

HttpWorker::HttpWorker(
    tcp::acceptor &acceptor,
    log_callback_t *logCallback)
    : mAcceptor(acceptor)
    , mLogCallback(logCallback)
{ /* ... */ }

void HttpWorker::start() {
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
      [this](beast::error_code ec) {
        if (ec) {
          accept();
        }
        else {
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
      [this](beast::error_code ec, std::size_t) {
        if (ec) {
          accept();
        }
        else {
          processRequest(mParser->get());
        }
      });
}

void HttpWorker::processRequest(http::request<http::string_body> const &req)
{
  switch (req.method())
  {
  case http::verb::get:
    sendResponse(req);
    break;
  default:
    sendBadResponse(
        http::status::bad_request,
        "Invalid request method '" + std::string(req.method_string()) + "'\r\n");
    break;
  }
}

void HttpWorker::sendResponse(http::request<http::string_body> const &req)
{
  if (mLogCallback != nullptr) {
    std::ostringstream ss;
    ss << mSocket.remote_endpoint().address().to_string() << ' '
       << req.target();
    (*mLogCallback)(ss.str());
  }
  if (req.method() == http::verb::get && req.target() == "/test") {
    mResponse.emplace();
    mResponse->result(http::status::ok);
    mResponse->set(http::field::server, std::string("Micro server ") + SERVER_VERSION);
    mResponse->set(http::field::content_type, "text/plain");
    mResponse->set("Access-Control-Allow-Origin", "*");
    mResponse->body() = "Hallo, Welt!";
    mResponse->prepare_payload();
    mSerializer.emplace(*mResponse);
    http::async_write(
        mSocket,
        *mSerializer,
        [this](boost::beast::error_code ec, std::size_t) {
          mSocket.shutdown(tcp::socket::shutdown_send, ec);
          mSerializer.reset();
          mResponse.reset();
          accept();
        });
  }
  else
  {
    sendBadResponse(http::status::not_found, "");
  }
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
      [this](beast::error_code ec, std::size_t) {
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
      [this](beast::error_code) {
        checkTimeout();
      });
}
