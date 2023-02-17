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

#ifndef __PATH_HPP__
#define __PATH_HPP__

#include <string>
#include <boost/beast/http/verb.hpp>

namespace trip
{
    typedef std::string path;

    struct path_method
    {
        trip::path path;
        boost::beast::http::verb verb;

        bool operator==(path_method const &o) const
        {
            return path == o.path && verb == o.verb;
        }
    };
}

namespace std {
    template<>
    struct hash<trip::path_method>
    {
        size_t operator()(trip::path_method const &k) const noexcept
        {
            size_t h1 = hash<string>{}(k.path);
            size_t h2 = hash<int>{}(static_cast<int>(k.verb));
            return h1 ^ (h2 << 1);
        }
    };
}

#endif
