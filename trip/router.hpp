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

#ifndef __ROUTER_HPP__
#define __ROUTER_HPP__

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

#include <boost/beast/http/string_body.hpp>
#include <boost/url.hpp>

#include "path.hpp"
#include "response_request.hpp"

namespace trip {
    namespace http = boost::beast::http;
    namespace url = boost::urls;

    class router
    {
        typedef std::function<response(const request &)> handler_t;
    private:
        std::unordered_map<path_method, handler_t> handlers;

    public:
        template<typename F, typename... Args>
        router &head(path path, F handler, Args... args)
        {
            handler_t callback = [&handler, &args...](const request &req)
            {
                return handler(req, args...);
            };
            handlers.emplace(path_method{path, http::verb::head}, callback);
            return *this;
        }

        template<typename F, typename... Args>
        router &get(path path, F handler, Args... args)
        {
            handler_t callback = [&handler, &args...](const request &req)
            {
                return handler(req, args...);
            };
            handlers.emplace(path_method{path, http::verb::get}, callback);
            return *this;
        }

        template<typename F, typename... Args>
        router &post(path path, F handler, Args... args)
        {
            handler_t callback = [&handler, &args...](const request &req)
            {
                return handler(req, args...);
            };
            handlers.emplace(path_method{path, http::verb::post}, callback);
            return *this;
        }

        response call(request const &req) const
        {
            url::result<url::url_view> target = url::parse_origin_form(req.target());
            if (target.has_error())
            {
                return trip::response{http::status::bad_request, "invalid target", "text/plain"};
            }
            const path_method pm{target->path(), req.method()};
            if (handlers.find(pm) != handlers.end())
            {
                return handlers.at(pm)(req);
            }
            return trip::response{http::status::not_found, target->path() + " not found", "text/plain"};
        }
    };

}

#endif // __ROUTER_HPP__
