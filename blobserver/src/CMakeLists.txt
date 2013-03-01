include_directories(.)

find_library(BOOST_THR NAMES boost_thread boost_thread-mt)
find_library(BOOST_SYS NAMES boost_system boost_system-mt)
find_library(BOOST_FS NAMES boost_filesystem boost_filesystem-mt)
find_library(BOOST_PO NAMES boost_program_options boost_program_options-mt)

SET(CITY_HASH_SRC_DIR "../../3rd/cityhash-1.1.0/")
SET(JSON_SPIRIT_SRC_DIR "../../3rd/json/")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CITY_HASH_SRC_DIR})
INCLUDE_DIRECTORIES(${JSON_SPIRIT_SRC_DIR})

SET(JSON_SPIRIT_CC ${JSON_SPIRIT_SRC_DIR}/json_spirit_reader.cc ${JSON_SPIRIT_SRC_DIR}/json_spirit_value.cc ${JSON_SPIRIT_SRC_DIR}/json_spirit_writer.cc)
SET(JSON_SPIRIT_H ${JSON_SPIRIT_SRC_DIR}/json_spirit_error_position.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_reader_template.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_utils.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_value.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_writer.h ${JSON_SPIRIT_SRC_DIR}/json_spirit.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_reader.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_stream_reader.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_writer_template.h)

ADD_LIBRARY(cityhash_lib STATIC ${CITY_HASH_SRC_DIR}/city)
ADD_LIBRARY(json_spirit_lib STATIC ${JSON_SPIRIT_CC} ${JSON_SPIRIT_H})

# -Wall -Wextra
add_definitions(-g -gstabs+)

add_executable(blobserver blobserver.cpp Config.cpp Blob.cpp BlobIndex.cpp connection.cpp mime_types.cpp request_handler.cpp server.cpp reply.cpp request_parser.cpp request.hpp header.hpp boost-multipart-form.cpp)
TARGET_LINK_LIBRARIES(blobserver ${BOOST_SYS} ${BOOST_FS} ${BOOST_THR} ${BOOST_PO} pthread cityhash_lib json_spirit_lib)