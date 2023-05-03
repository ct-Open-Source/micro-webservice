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

#include <boost/asio/ip/tcp.hpp>

#include "global.hpp"
#include "httpworker.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

http_worker::http_worker(
    tcp::acceptor &acceptor,
    const trip::router &router,
    log_callback_t *logCallback)
    : acceptor_(acceptor)
    , router_(router)
    , log_callback_(logCallback)
{
  /* ... */
}

void http_worker::start()
{
  accept();
  check_timeout();
}

void http_worker::accept()
{
  beast::error_code ec;
  socket_.close(ec);
  buffer_.consume(buffer_.size());
  acceptor_.async_accept(
      socket_,
      [this](beast::error_code ec)
      {
        if (ec)
        {
          accept();
        }
        else
        {
          // req_timeout_.expires_after(Timeout);
          read_request();
        }
      });
}

void http_worker::read_request()
{
  parser_.emplace();
  http::async_read(
      socket_,
      buffer_,
      *parser_,
      [this](beast::error_code ec, std::size_t)
      {
        if (ec)
        {
          accept();
        }
        else
        {
          process_request(parser_->get());
        }
      });
}

void http_worker::process_request(const http::request<http::string_body> &req)
{
  if (log_callback_ != nullptr)
  {
    std::ostringstream ss;
    ss << socket_.remote_endpoint().address().to_string() << ' '
       << req.method() << ' '
       << req.target();
    (*log_callback_)(ss.str());
  }
  trip::response response = router_.execute(req);
  if (response.status == http::status::ok)
  {
    send_response(response.body, response.mime_type);
  }
  else
  {
    send_error_response(response.status, response.body, response.mime_type);
  }
}

void http_worker::send()
{
  response_->set(http::field::server, SERVER_INFO);
  response_->set(http::field::access_control_allow_origin, "*");
  response_->prepare_payload();
  serializer_.emplace(*response_);
  http::async_write(
      socket_,
      *serializer_,
      [this](beast::error_code ec, std::size_t)
      {
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        serializer_.reset();
        response_.reset();
        accept();
      });
}

void http_worker::send_response(const std::string &body, const std::string &mimetype)
{
  response_.emplace();
  response_->result(http::status::ok);
  response_->set(http::field::content_type, mimetype);
  response_->body() = body;
  send();
}

void http_worker::send_error_response(http::status status, const std::string &error, const std::string &mimetype)
{
  response_.emplace();
  response_->result(status);
  response_->set(http::field::content_type, mimetype);
  response_->body() = error;
  send();
}

void http_worker::check_timeout()
{
  if (req_timeout_.expiry() <= std::chrono::steady_clock::now())
  {
    socket_.close();
    req_timeout_.expires_at(std::chrono::steady_clock::time_point::max());
  }
  req_timeout_.async_wait(
      [this](beast::error_code)
      {
        check_timeout();
      });
}
