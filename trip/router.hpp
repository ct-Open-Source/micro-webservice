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

#include <boost/beast/http/string_body.hpp>
#include <boost/regex.hpp>
#include <boost/url.hpp>

#include "response_request.hpp"

namespace trip
{
    namespace http = boost::beast::http;
    namespace url = boost::urls;

    class router
    {
        typedef std::function<response(request const &, boost::smatch const &)> handler_t;
        struct route
        {
            http::verb const verb;
            boost::regex const endpoint;
            handler_t const handler;
            route() = delete;
            route(http::verb const &verb, boost::regex const &endpoint, handler_t const &handler)
                : verb(verb), endpoint(endpoint), handler(handler)
            {
            }
        };

    public:
        template <typename F>
        router &head(boost::regex endpoint, F handler) noexcept
        {
            routes_.emplace_back(http::verb::head, endpoint, handler);
            return *this;
        }

        template <typename F>
        router &get(boost::regex endpoint, F handler) noexcept
        {
            routes_.emplace_back(http::verb::get, endpoint, handler);
            return *this;
        }

        template <typename F>
        router &post(boost::regex endpoint, F handler) noexcept
        {
            routes_.emplace_back(http::verb::post, endpoint, handler);
            return *this;
        }

        response execute(request const &req) const
        {
            url::result<url::url_view> target = url::parse_origin_form(req.target());
            if (target.has_error())
            {
                return trip::response{http::status::bad_request, "invalid target", "text/plain"};
            }
            boost::smatch match;
            auto r = routes_.cbegin();
            while (r != routes_.cend())
            {
                if (r->verb == req.method() && boost::regex_match(target->path(), match, r->endpoint))
                {
                    break;
                }
                ++r;
            }
            if (r != routes_.cend())
            {
                return r->handler(req, match);
            }
            return trip::response{http::status::not_found, target->path() + " not found", "text/plain"};
        }

    private:
        std::vector<route> routes_;
    };

}

#endif // __TRIP_ROUTER_HPP__
