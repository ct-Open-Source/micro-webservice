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

#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

#include <boost/beast/http/string_body.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <boost/regex.hpp>

#include "number_theory.hpp"
#include "handlers.hpp"

namespace pt = boost::property_tree;
namespace beast = boost::beast;
namespace http = beast::http;
namespace chrono = std::chrono;


static std::string convert_float(const std::string &json_str)
{
    static boost::regex re("\\\"([0-9]+\\.?[0-9]*)\\\"");
    return boost::regex_replace(json_str, re, "$1", boost::match_default);
}


static std::string convert_bool(const std::string &json_str)
{
    static boost::regex re("\\\"(true|false)\\\"");
    return boost::regex_replace(json_str, re, "$1");
}


warp::response handle_prime(const warp::request &req)
{
    pt::ptree request;
    std::stringstream iss;
    iss << req.body();
    try
    {
        pt::read_json(iss, request);
    }
    catch (pt::ptree_error e)
    {
        return warp::response{http::status::bad_request, "{\"error\": \"" + std::string(e.what()) + "\""};
    }
    if (request.find("number") == request.not_found())
    {
        return warp::response{http::status::bad_request, "{\"error\": \"field \\\"number\\\" is missing\"}"};
    }
    bigint x{0};
    try
    {
        x.assign(request.get<std::string>("number"));
    }
    catch (...)
    {
        return warp::response{http::status::bad_request, "{\"error\": \"field \\\"number\\\" must contain a positive integer number\"}"};
    }
    auto t0 = chrono::high_resolution_clock::now();
    bool isprime = primality::is_prime(x) != primality::composite;
    auto t1 = chrono::high_resolution_clock::now();
    auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
    pt::ptree response;
#ifndef WITH_REGEX_REPLACE
    response.put("elapsed_msecs", "[elapsed_msecs]");
    response.put("isprime", "[isprime]");
#else
    response.put("elapsed_msecs", 1e3 * dt.count());
    response.put("isprime", isprime);
#endif
    response.put("number", x.convert_to<std::string>());
    std::ostringstream ss;
    pt::write_json(ss, response, true);
    std::string responseStr = ss.str();
#ifndef WITH_REGEX_REPLACE
    boost::replace_all(responseStr, "\"[elapsed_msecs]\"", std::to_string(1e3 * dt.count()));
    boost::replace_all(responseStr, "\"[isprime]\"", isprime ? "true" : "false");
#else
    responseStr = convert_bool(responseStr);
    responseStr = convert_float(responseStr);
#endif
    return warp::response{http::status::ok, responseStr};
}


warp::response handle_factor(const warp::request &req)
{
    pt::ptree request;
    std::stringstream iss;
    iss << req.body();
    try
    {
        pt::read_json(iss, request);
    }
    catch (pt::ptree_error e)
    {
        return warp::response{http::status::bad_request, "{\"error\": \"" + std::string(e.what()) + "\""};
    }
    if (request.find("number") == request.not_found())
    {
        return warp::response{http::status::bad_request, "{\"error\": \"field \\\"number\\\" is missing\"}"};
    }
    bigint x{0};
    try
    {
        x.assign(request.get<std::string>("number"));
    }
    catch (...)
    {
        return warp::response{http::status::bad_request, "{\"error\": \"field \\\"number\\\" must contain a positive integer number\"}"};
    }
    auto t0 = chrono::high_resolution_clock::now();
    std::vector<bigint> factors = primality::factors(x);
    auto t1 = chrono::high_resolution_clock::now();
    auto dt = chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
    pt::ptree factors_child;
    for (auto const &f : factors)
    {
        pt::ptree child;
        child.put("", f);
        factors_child.push_back(std::make_pair("", child));
    }
    pt::ptree response;
    response.put("number", x.convert_to<std::string>());
    if (factors.empty())
    {
        response.put("factors", "[factors]");
    }
    else
    {
        response.add_child("factors", factors_child);
    }
    response.put("elapsed_msecs", 1e3 * dt.count());
    response.put("isprime", factors.empty());
    std::ostringstream ss;
    pt::write_json(ss, response, true);
    std::string responseStr = ss.str();
    responseStr = convert_bool(responseStr);
    responseStr = convert_float(responseStr);
    boost::replace_all(responseStr, std::string("\"[factors]\""), "[]");
    return warp::response{http::status::ok, responseStr, "application/json"};
}
