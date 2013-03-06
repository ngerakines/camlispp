
OPTION(ENABLE_DEBUG "Allow debug flags to be used." OFF)
OPTION(ENABLE_DUMP "Enable code to dump objects" OFF)
OPTION(ENABLE_DEBUG_LOAD "Enable loading of arbitrary files on startup" OFF)
OPTION(ENABLE_MD5 "Enable use of md5" ON)
OPTION(ENABLE_STATIC "Enable static site" ON)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/blobserver/config.h.cmake ${CMAKE_BINARY_DIR}/blobserver/src/config.h)

add_subdirectory(src)
