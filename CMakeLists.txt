cmake_minimum_required(VERSION 2.6)

add_subdirectory(cmake)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include (TargetDistclean)

project(camlispp CXX)

FIND_PACKAGE(Boost 1.50 REQUIRED)

add_subdirectory(blobserver)

