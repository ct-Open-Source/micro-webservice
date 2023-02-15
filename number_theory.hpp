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

#ifndef __number_theory_hpp__
#define __number_theory_hpp__

#include <boost/random.hpp>
#include <vector>
#include <chrono>

namespace number_theory
{

    template <typename INT>
    class prime
    {
        static INT power(INT x, INT y, INT p)
        {
            INT res = 1;
            x = x % p;
            while (y > 0)
            {
                if (y & 1)
                {
                    res = (res * x) % p;
                }
                y /= 2;
                x = (x * x) % p;
            }
            return res;
        }

        static bool mr_prime(INT d, INT n)
        {
            typedef boost::random::independent_bits_engine<boost::random::mt19937, 16, INT> generator_type;
            generator_type random_number;
            random_number.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

            INT a = 2 + random_number() % (n - 4);
            INT x = power(a, d, n);
            if (x == 1 || x == n - 1)
            {
                return true;
            }
            while (d != n - 1)
            {
                x = (x * x) % n;
                d *= 2;
                if (x == 1)
                {
                    return false;
                }
                if (x == n - 1)
                {
                    return true;
                }
            }
            return false;
        }

    public:
        static std::vector<INT> factors(INT x)
        {
            std::vector<INT> result;
            if (is_prime(x, 5))
            {
                return result;
            }
            INT z = x;
            while (z > 1)
            {
                INT i = 2;
                bool found = false;
                INT p;
                while (i*i <= z and !found)
                {
                    if (z % i == 0)
                    {
                        found = true;
                        p = i;
                    }
                    else
                    {
                        ++i;
                    }
                }
                if (!found)
                {
                    p = z;
                }
                result.push_back(p);
                z /= p;
            }
            return result;
        }

        static bool is_prime(INT n, int k)
        {
            if (n <= 1 || n == 4)
            {
                return false;
            }
            if (n <= 3)
            {
                return true;
            }
            INT d = n - 1;
            while (d % 2 == 0)
            {
                d /= 2;
            }
            for (int i = 0; i < k; i++)
            {
                if (!mr_prime(d, n))
                {
                    return false;
                }
            }
            return true;
        }
    };

}
#endif // __number_theory_hpp__