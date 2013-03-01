
OPTION(ENABLE_DEBUG "Allow debug flags to be used." OFF)
OPTION(ENABLE_DUMP "Enable code to dump objects" OFF)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/blobserver/config.h.cmake ${CMAKE_BINARY_DIR}/blobserver/src/config.h)

add_subdirectory(src)
