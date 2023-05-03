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

#ifndef __NUMBER_THEORY_HPP__
#define __NUMBER_THEORY_HPP__

#include <vector>
#include <chrono>

#include <boost/random.hpp>
#include <boost/multiprecision/gmp.hpp>

typedef boost::multiprecision::mpz_int bigint;

class primality
{
    static bigint power(bigint x, bigint y, bigint p)
    {
        bigint res = 1;
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

    static bool mr_prime(bigint d, bigint n, bigint a)
    {
        bigint x = power(a, d, n);
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
    enum _primality_result
    {
        prime,
        probably_prime,
        composite
    };
    typedef enum _primality_result primality_result;

    static std::vector<bigint> factors(bigint x)
    {
        std::vector<bigint> result;
        if (is_prime(x) != composite == primality::primality_result::prime)
        {
            return result;
        }
        bigint z = x;
        while (z > 1)
        {
            bigint i = 2;
            bool found = false;
            bigint p;
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

    template<typename T>
    static T sqr(T x)
    {
        return x * x;
    }

    static long log2(bigint x)
    {
        long exp;
        (void)mpz_get_d_2exp(&exp, x.backend().data());
        return exp;
    }

    static primality_result is_prime(bigint n, bool fast = true)
    {
        if (n <= 1 || n == 4)
        {
            return composite;
        }
        if (n <= 5)
        {
            return prime;
        }
        std::vector<bigint> A;
        primality_result result = prime;
        if (n < 2047)
        {
            A = {2};
        }
        else if (n < 1'373'653)
        {
            A = {2, 3};
        }
        else if (n < 9'080'191)
        {
            A = {31, 73};
        }
        else if (n < 25'326'001)
        {
            A = {2, 3, 5};
        }
        else if (n < 3'215'031'751)
        {
            A = {2, 3, 5, 7};
        }
        else if (n < 4'759'123'141)
        {
            A = {2, 7, 61};
        }
        else if (n < 1'122'004'669'633)
        {
            A = {2, 13, 23, 1662803};
        }
        else if (n < 2'152'302'898'747)
        {
            A = {2, 3, 5, 7, 11};
        }
        else if (n < 3'474'749'660'383)
        {
            A = {2, 3, 5, 7, 11, 13};
        }
        else if (n < 341'550'071'728'321)
        {
            A = {2, 3, 5, 7, 11, 13, 17};
        }
        else if (n < 3'825'123'056'546'413'051)
        {
            A = {2, 3, 5, 7, 11, 13, 17, 19, 23};
        }
        else if (n < bigint{"18446744073709551616"})
        {
            A = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
        }
        else if (n < bigint{"318665857834031151167461"})
        {
            A = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
        }
        else if (n < bigint{"3317044064679887385961981"})
        {
            A = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
        }
        else
        {
            const size_t amax = 2UL * std::max(2UL, static_cast<size_t>(sqr(double(log2(n)) / M_LN2)));
            if (fast)
            {
                A = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
                const bigint amin = A.back();
                boost::random::mersenne_twister_engine<uint64_t, 64UL, 624UL, 397UL, 31UL, 2567483615U, 11UL, 4294967295U, 7UL, 2636928640U, 15UL, 4022730752U, 18UL, 1812433253U> rng;
                rng.seed(static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()));
                constexpr size_t ITERATIONS = 5;
                A.reserve(A.size() + ITERATIONS);
                for (size_t i = 0; i < ITERATIONS; ++i)
                {
                    A.push_back(amin + rng() % (amax - amin));
                }
                result = probably_prime;
            }
            else
            {
                A.reserve(amax);
                for (bigint a = 2; a < amax; ++a)
                {
                    A.push_back(a);
                }
            }
        }
        assert(A.size() > 0);
        bigint d = n - 1;
        while (d % 2 == 0)
        {
            d /= 2;
        }
        for (const bigint a : A)
        {
            if (mr_prime(d, n, a) == false)
            {
                return composite;
            }
        }
        return result;
    }
};

#endif // __NUMBER_THEORY_HPP__
