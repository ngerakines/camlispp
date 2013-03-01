#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#if defined ENABLE_DEBUG
#include <iostream>
#endif

#include "config.h"

#if defined ENABLE_DEBUG
#define LOG_INFO(X) std::cout << X;
#define LOG_ERROR(X) std::cerr << X;
#else
#define LOG_INFO(x)
#define LOG_ERROR(x)
#endif


#endif // LOGGER_HPP_INCLUDED
