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

#include <string>
#include <unordered_map>
#include <functional>

#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/string_body.hpp>

namespace warp {
    typedef std::string path;

    struct path_method
    {
        warp::path path;
        boost::beast::http::verb verb;
    };

    bool operator==(const path_method& lhs, const path_method& rhs);
}

namespace std {
    template<>
    struct hash<warp::path_method>
    {
        size_t operator()(const warp::path_method & k) const noexcept
        {
            size_t h1 = hash<string>{}(k.path);
            size_t h2 = hash<int>{}(static_cast<int>(k.verb));
            return h1 ^ (h2 << 1);
        }
    };
}

namespace warp {
    namespace http = boost::beast::http;

    struct response
    {
        http::status status;
        std::string body;
        std::string mime_type;

        response() = delete;
    };

    typedef http::request<http::string_body> request;

    class router
    {
    private:
        std::unordered_map<path_method, std::function<response(const request &)>> handlers;

    public:
        template<typename F, typename... Args>
        router &head(path path, F handler, Args&&... args)
        {
            std::function<response(const request &)>
            callback = [handler, args...](const request &req)
            {
                return handler(req, args...);
            };
            handlers.emplace(path_method{path, http::verb::head}, callback);
            return *this;
        }

        template<typename F, typename... Args>
        router &get(path path, F handler, Args&&... args)
        {
            std::function<response(const request &)>
            callback = [handler, args...](const request &req)
            {
                return handler(req, args...);
            };
            handlers.emplace(path_method{path, http::verb::get}, callback);
            return *this;
        }

        template<typename F, typename... Args>
        router &post(path path, F handler, Args&&... args)
        {
            std::function<response(const request &)>
            callback = [handler, args...](const request &req)
            {
                return handler(req, args...);
            };
            handlers.emplace(path_method{path, http::verb::post}, callback);
            return *this;
        }

        response call(http::verb method, path path, const request &req) const
        {
            const path_method pm{path, method};
            if (handlers.find(pm) != handlers.end())
            {
                return handlers.at(pm)(req);
            }
            return response();
        }
    };

}

#endif // __ROUTER_HPP__
