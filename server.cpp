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
#include <thread>
#include <mutex>

#include "helper.hpp"
#include "server.hpp"
#include "httpworker.hpp"

using tcp = boost::asio::ip::tcp;
namespace net = boost::asio;

void hello()
{
  std::cout << "c't demo: micro webservice " << SERVER_VERSION << " - Copyright (c) 2023 Oliver Lau <ola@ct.de>" << std::endl
            << std::endl;
}

void usage()
{
  std::cout << "Usage:" << std::endl
            << "  micro-webservice <ip> <port> <num_workers> <num_threads>" << std::endl
            << std::endl
            << "for example:" << std::endl
            << "  micro-webservice 127.0.0.1 31377 100 8" << std::endl
            << std::endl;
}

int main(int argc, const char *argv[])
{
  if (argc < 5)
  {
    usage();
    return EXIT_FAILURE;
  }
  hello();

  auto host = net::ip::make_address(argv[1]);
  uint16_t port = static_cast<uint16_t>(std::atoi(argv[2]));
  int numWorkers = std::max<int>(1, std::atoi(argv[3])); // std::thread::hardware_concurrency();
  int numThreads = std::max<int>(1, std::atoi(argv[4])); // std::thread::hardware_concurrency();

  boost::asio::io_context ioc;
  tcp::acceptor acceptor{ioc, {host, port}};
  std::list<HttpWorker> workers;

  uint64_t connId = 0;

  std::mutex logMtx;
  HttpWorker::log_callback_t logger = [&logMtx, &connId](const std::string &msg)
  {
    std::lock_guard<std::mutex> lock(logMtx);
    std::cout << std::chrono::system_clock::now() << ' '
              << msg << ' ' << (connId++) << std::endl;
  };

  for (int i = 0; i < numWorkers; ++i)
  {
    workers.emplace_back(acceptor, nullptr);
    workers.back().start();
  }
  
  std::vector<std::thread> threads;
  threads.reserve(size_t(numThreads - 1));
  for (auto i = 0; i < numThreads - 1; ++i)
  {
    threads.emplace_back(
        [&ioc]
        {
          ioc.run();
        });
  }

  std::cout << numWorkers << " workers" 
     << (numThreads > 1 ? " in " + std::to_string(numThreads) + " threads" : " in 1 thread")
     << " listening on " << host << ':' << port << " ..."
     << std::endl;

  ioc.run();

  for (auto &t : threads)
  {
    t.join();
  }

  return EXIT_SUCCESS;
}
