#ifndef LOGGER_HPP_INCLUDED
#define LOGGER_HPP_INCLUDED

#include <iostream>

#include "config.h"

#if defined ENABLE_DUMP
#define LOG_INFO(X) std::cout << X;
#define LOG_ERROR(X) std::cerr << X;
#else
#define LOG_INFO(X)
#define LOG_ERROR(X)
#endif

#endif // LOGGER_HPP_INCLUDED
