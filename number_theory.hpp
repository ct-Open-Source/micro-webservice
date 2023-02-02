#ifndef __number_theory_hpp__
#define __number_theory_hpp__

#include <boost/random.hpp>
#include <vector>

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
            INT i = 1;
            while (i * i <= x)
            {
                if (x % i == 0)
                {
                    result.push_back(i);
                    if (x / i != i)
                    {
                        result.push_back(x / i);
                    }
                }
                ++i;
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
