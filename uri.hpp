/*
 Copyright © 2019, 2023 Oliver Lau <ola@ct.de>, Heise Medien GmbH & Co. KG - Redaktion c't

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

#ifndef __URI_HPP__
#define __URI_HPP__

#include <string>
#include <map>
#include <cstdint>

class URI
{
public:
  typedef uint16_t port_type;
  URI() = default;
  URI(URI &&) = delete;
  URI(const std::string &uri);
  void parse(const std::string &uri);
  void parseTarget(const std::string &target);
  bool isValid() const;
  const std::string &scheme() const;
  const std::string &username() const;
  const std::string &password() const;
  const std::string &host() const;
  port_type port() const;
  const std::string &path() const;
  const std::map<std::string, std::string> &query();
  const std::string &fragment() const;

private:
  bool mIsValid{false};
  std::string mScheme;
  std::string mUsername;
  std::string mPassword;
  std::string mHost;
  port_type mPort{0};
  std::string mPath;
  std::map<std::string, std::string> mQuery;
  std::string mFragment;
};

#endif // __URI_HPP__
