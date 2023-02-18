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

#ifndef __TRIP_RESPONSE_REQUEST_HPP__
#define __TRIP_RESPONSE_REQUEST_HPP__

#include <string>
#include <boost/beast/http.hpp>

namespace trip
{
    namespace http = boost::beast::http;

    struct response
    {
        http::status status;
        std::string body;
        std::string mime_type = "application/json";
    };

    typedef http::request<http::string_body> request;
    typedef http::status status;
}

#endif // __TRIP_RESPONSE_REQUEST_HPP__
