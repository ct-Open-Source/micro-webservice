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

#include <boost/regex.hpp>

#include "helper.hpp"

std::string convert_float(std::string const &json_str)
{
    static boost::regex re("\\\"([0-9]+\\.?[0-9]*)\\\"");
    return boost::regex_replace(json_str, re, "$1");
}


std::string convert_bool(std::string const &json_str)
{
    static boost::regex re("\\\"(true|false)\\\"");
    return boost::regex_replace(json_str, re, "$1");
}
