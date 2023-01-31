#ifndef __helper_hpp__
#define __helper_hpp__

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

#endif // __helper_hpp__