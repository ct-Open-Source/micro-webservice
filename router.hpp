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
        path path;
        boost::beast::http::verb verb;
    };

    bool operator==(const path_method& lhs, const path_method& rhs)
    {
        return lhs.path == rhs.path && lhs.verb == rhs.verb;
    }

}

namespace std {
    template<>
    struct std::hash<warp::path_method>
    {
        std::size_t operator()(const warp::path_method & k) const noexcept
        {
            std::size_t h1 = std::hash<std::string>{}(k.path);
            std::size_t h2 = std::hash<int>{}(static_cast<int>(k.verb));
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
