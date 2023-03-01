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

#ifndef __TRIP_ROUTER_HPP__
#define __TRIP_ROUTER_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <regex>
#include <mutex>
#include <memory>

#include <boost/beast/http/string_body.hpp>
#include <boost/url.hpp>

#include "response_request.hpp"

namespace trip
{
    namespace http = boost::beast::http;
    namespace url = boost::urls;
    namespace asio = boost::asio;

    class router
    {
        typedef std::function<response(request const &, std::regex const &)> handler_t;
        struct route
        {
            http::verb const verb;
            std::regex const endpoint;
            handler_t const &handler;
            std::unique_ptr<std::mutex> mutex;
            route() = delete;
            route(http::verb const &verb, std::regex const &endpoint, handler_t const &handler, bool serialize)
            : verb(verb)
            , endpoint(endpoint)
            , handler(handler)
            {
                if (serialize)
                {
                    mutex = std::make_unique<std::mutex>();
                }
            }
        };

    public:
        router &get(std::regex endpoint, handler_t handler, bool serialize) noexcept
        {
            routes_.emplace_back(http::verb::get, endpoint, handler, serialize);
            return *this;
        }

        router &post(std::regex endpoint, handler_t handler, bool serialize) noexcept
        {
            routes_.emplace_back(http::verb::post, endpoint, handler, serialize);
            return *this;
        }

        router &options(std::regex endpoint, handler_t handler, bool serialize) noexcept
        {
            routes_.emplace_back(http::verb::options, endpoint, handler, serialize);
            return *this;
        }

        router &head(std::regex endpoint, handler_t handler, bool serialize) noexcept
        {
            routes_.emplace_back(http::verb::head, endpoint, handler, serialize);
            return *this;
        }

        router &put(std::regex endpoint, handler_t handler, bool serialize) noexcept
        {
            routes_.emplace_back(http::verb::put, endpoint, handler, serialize);
            return *this;
        }

        router &patch(std::regex endpoint, handler_t handler, bool serialize) noexcept
        {
            routes_.emplace_back(http::verb::patch, endpoint, handler, serialize);
            return *this;
        }

        response execute(request const &req) const
        {
            url::result<url::url_view> target = url::parse_origin_form(req.target());
            if (target.has_error())
            {
                return trip::response{http::status::bad_request, "invalid target", "text/plain"};
            }
            for (route const &r : routes_)
            {
                if (r.verb == req.method() && std::regex_match(target->path(), r.endpoint))
                {
                    if (r.mutex)
                    {
                        std::lock_guard<std::mutex> lock(*r.mutex);
                        return r.handler(req, r.endpoint);
                    }
                    return r.handler(req, r.endpoint);
                }
            }
            return trip::response{http::status::not_found, target->path() + " not found", "text/plain"};
        }

    private:
        std::vector<route> routes_;
    };

}

#endif // __TRIP_ROUTER_HPP__
