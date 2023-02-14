#ifndef __HANDLERS_HPP__
#define __HANDLERS_HPP__

#include "router.hpp"

extern warp::response handle_factor(const warp::request &);
extern warp::response handle_prime(const warp::request &);

#endif
