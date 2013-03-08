#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#include "config.h"
#include "prettyprint.hpp"

#if defined ENABLE_DUMP
#define LOG_INFO(X) std::cout << X;
#define LOG_ERROR(X) std::cerr << X;
#else
#define LOG_INFO(X)
#define LOG_ERROR(X)
#endif

#endif // LOGGER_HPP_INCLUDED
