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

#ifndef __HELPER_HPP__
#define __HELPER_HPP__

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

template<typename Clock, typename Duration>
std::ostream &operator<<(std::ostream &stream, const std::chrono::time_point<Clock, Duration> &time_point)
{
  const time_t time = Clock::to_time_t(time_point);
#if __GNUC__ > 4 || ((__GNUC__ == 4) && __GNUC_MINOR__ > 8 && __GNUC_REVISION__ > 1)
  struct tm tm;
  localtime_r(&time, &tm);
  return stream << std::put_time(&tm, "%c");
#else
  char buffer[26];
  ctime_r(&time, buffer);
  buffer[24] = '\0';
  return stream << buffer;
#endif
}

extern std::string convert_float(std::string const &json_str);
extern std::string convert_bool(std::string const &json_str);


#endif // __HELPER_HPP__
