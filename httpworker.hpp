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

#ifndef __httpworker_hpp__
#define __httpworker_hpp__

#include <string>
#include <ctime>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/optional/optional.hpp>
#include <boost/function.hpp>

namespace beast = boost::beast;
namespace http = beast::http;

class HttpWorker
{
  using tcp = boost::asio::ip::tcp;

public:
  typedef boost::function<void(const std::string&)> log_callback_t;
  HttpWorker(HttpWorker const &) = delete;
  HttpWorker& operator=(HttpWorker const &) = delete;
  HttpWorker(
      tcp::acceptor &acceptor,
      log_callback_t *logFn = nullptr);
  void start();

  static constexpr std::chrono::seconds Timeout{60};

private:
  tcp::acceptor &mAcceptor;
  tcp::socket mSocket{mAcceptor.get_executor()};
  beast::flat_buffer mBuffer;
  boost::optional<http::request_parser<http::string_body>> mParser;
  boost::asio::basic_waitable_timer<std::chrono::steady_clock> mReqTimeout{
    mAcceptor.get_executor(),
    (std::chrono::steady_clock::time_point::max)()};
  boost::optional<http::response<http::string_body>> mResponse;
  boost::optional<http::response_serializer<http::string_body>> mSerializer;
  log_callback_t *mLogCallback;

  void accept();
  void readRequest();
  void sendResponse(http::request<http::string_body> const &);
  void processRequest(http::request<http::string_body> const &req);
  void sendBadResponse(http::status status, const std::string &error);
  void checkTimeout();
};

#endif // __httpworker_hpp__
