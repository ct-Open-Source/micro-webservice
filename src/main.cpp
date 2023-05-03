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
#include <string>
#include <memory>
#include <sstream>
#include <list>
#include <vector>
#include <thread>
#include <memory>
#include <regex>

#include <boost/lexical_cast.hpp>
#include <boost/asio/signal_set.hpp>

#include "global.hpp"
#include "helper.hpp"
#include "httpworker.hpp"
#include "handlers.hpp"
#include "trip/router.hpp"

#ifndef NDEBUG
const char *DEFAULT_HOST = "0.0.0.0";
#else
const char *DEFAULT_HOST = "127.0.0.1";
#endif
constexpr uint16_t DEFAULT_PORT = 31337U;

using tcp = boost::asio::ip::tcp;
namespace net = boost::asio;

void hello()
{
  std::cout << "c't demo: " << SERVER_INFO << std::endl
            << std::endl;
}

void usage()
{
  std::cout << "Usage:" << std::endl
            << "  micro-webservice [<ip> <port> <num_workers> <num_threads>]" << std::endl
            << std::endl
            << "for example:" << std::endl
            << "  micro-webservice 0.0.0.0 8081 2 2" << std::endl
            << std::endl
            << "or just:" << std::endl
            << "  micro-webservice" << std::endl
            << std::endl
            << "to use the defaults: " << DEFAULT_HOST << " " << DEFAULT_PORT << " N N" << std::endl
            << "where N stands for the number of CPU cores ("
            << std::thread::hardware_concurrency() << ")." << std::endl
            << std::endl;
}

int main(int argc, const char *argv[])
{
  hello();
  if (!(argc == 1 || argc == 5))
  {
    usage();
    return EXIT_FAILURE;
  }

  net::ip::address host = net::ip::make_address(DEFAULT_HOST);
  uint16_t port = DEFAULT_PORT;
  unsigned int num_workers = std::thread::hardware_concurrency();
  unsigned int num_threads = num_workers;
  if (argc == 5)
  {
    try
    {
      host = net::ip::make_address(argv[1]);
      port = boost::lexical_cast<uint16_t>(argv[2]);
      num_workers = std::max(1U, boost::lexical_cast<unsigned int>(argv[3]));
      num_threads = std::max(1U, boost::lexical_cast<unsigned int>(argv[4]));
    }
    catch (boost::exception const&)
    {
      usage();
      return EXIT_FAILURE;
    }
  }

  boost::asio::io_context ioc;
  tcp::acceptor acceptor{ioc, {host, port}};

  std::mutex log_mtx;
  http_worker::log_callback_t logger = [&log_mtx](const std::string &msg)
  {
    std::lock_guard<std::mutex> lock(log_mtx);
    std::cout << std::chrono::system_clock::now() << ' '
              << msg << std::endl;
  };

  trip::router router;
  router
    .post(std::regex("/prime"), handle_prime{}, false)
    .post(std::regex("/factor"), handle_factor{}, false)
    .get(std::regex("/countdown"), handle_countdown{10}, true)
    .get(std::regex("/square/(-?\\d+)"), handle_square{}, false)
    .get(std::regex("/mult/(-?\\d+)/(-?\\d+)"), handle_mult{}, false)
    .get(std::regex("/ioc"), handle_with_ioc{ioc}, false)
  ;

  std::list<http_worker> workers;
  for (auto i = 0U; i < num_workers; ++i)
  {
    workers.emplace_back(acceptor, router, &logger);
    workers.back().start();
  }
  
  std::vector<std::thread> threads;
  threads.reserve(num_threads - 1);
  for (auto i = 0U; i < num_threads - 1; ++i)
  {
    threads.emplace_back(
        [&ioc]
        {
          ioc.run();
        });
  }

  net::signal_set signals(ioc, SIGINT, SIGTERM);
  signals.async_wait(
      [&ioc](boost::system::error_code const&, int)
      {
        ioc.stop();
      });

  std::cout << (num_workers > 1 ? std::to_string(num_workers) + " workers" : " 1 worker")
     << (num_threads > 1 ? " in " + std::to_string(num_threads) + " threads" : " in 1 thread")
     << " listening on " << host << ':' << port << " ..."
     << std::endl;

  ioc.run();

  for (auto &t : threads)
  {
    t.join();
  }

  return EXIT_SUCCESS;
}
