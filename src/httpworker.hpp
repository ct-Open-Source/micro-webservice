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

#ifndef __HTTP_WORKER_HPP__
#define __HTTP_WORKER_HPP__

#include <string>
#include <chrono>
#include <functional>
#include <optional>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/optional/optional.hpp>

#include "trip/router.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

class http_worker
{
  using tcp = boost::asio::ip::tcp;

public:
  typedef std::function<void(const std::string&)> log_callback_t;

  http_worker(http_worker const &) = delete;
  http_worker& operator=(http_worker const &) = delete;
  http_worker(
      tcp::acceptor &acceptor,
      const trip::router &router,
      log_callback_t *logFn = nullptr);
  void start();

  static constexpr std::chrono::seconds Timeout{60};

private:
  tcp::acceptor &acceptor_;
  trip::router const &router_;
  tcp::socket socket_{acceptor_.get_executor()};
  beast::flat_buffer buffer_;
  std::optional<http::request_parser<http::string_body>> parser_;
  boost::asio::basic_waitable_timer<std::chrono::steady_clock> req_timeout_{
    acceptor_.get_executor(),
    (std::chrono::steady_clock::time_point::max)()};
  std::optional<http::response<http::string_body>> response_;
  std::optional<http::response_serializer<http::string_body>> serializer_;
  log_callback_t *log_callback_;

  void accept();
  void read_request();
  void process_request(const http::request<http::string_body> &req);
  void send();
  void send_response(const std::string &body, const std::string &mimetype);
  void send_error_response(http::status status, const std::string &error, const std::string &mimetype);
  void check_timeout();
};

#endif // __HTTP_WORKER_HPP__
