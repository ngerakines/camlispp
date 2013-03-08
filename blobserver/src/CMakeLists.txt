include_directories(.)

find_library(BOOST_THR NAMES boost_thread boost_thread-mt)
find_library(BOOST_SYS NAMES boost_system boost_system-mt)
find_library(BOOST_FS NAMES boost_filesystem boost_filesystem-mt)
find_library(BOOST_PO NAMES boost_program_options boost_program_options-mt)
find_library(BOOST_SE NAMES boost_serialization boost_serialization-mt)
find_library(BOOST_IO NAMES boost_iostreams boost_iostreams-mt)
find_library(CRYPTOPP NAMES cryptopp)

SET(JSON_SPIRIT_SRC_DIR "../../3rd/json/")
SET(ZOPFLI_SRC_DIR "../../3rd/zopfli/")

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${JSON_SPIRIT_SRC_DIR})
INCLUDE_DIRECTORIES(${ZOPFLI_SRC_DIR})

SET(JSON_SPIRIT_CC ${JSON_SPIRIT_SRC_DIR}/json_spirit_reader.cc ${JSON_SPIRIT_SRC_DIR}/json_spirit_value.cc ${JSON_SPIRIT_SRC_DIR}/json_spirit_writer.cc)
SET(JSON_SPIRIT_H ${JSON_SPIRIT_SRC_DIR}/json_spirit_error_position.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_reader_template.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_utils.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_value.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_writer.h ${JSON_SPIRIT_SRC_DIR}/json_spirit.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_reader.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_stream_reader.h ${JSON_SPIRIT_SRC_DIR}/json_spirit_writer_template.h)

ADD_LIBRARY(json_spirit_lib STATIC ${JSON_SPIRIT_CC} ${JSON_SPIRIT_H})
ADD_LIBRARY(zopfli_lib STATIC ${ZOPFLI_SRC_DIR}blocksplitter.cc ${ZOPFLI_SRC_DIR}cache.cc ${ZOPFLI_SRC_DIR}deflate.cc ${ZOPFLI_SRC_DIR}gzip_container.cc ${ZOPFLI_SRC_DIR}hash.cc ${ZOPFLI_SRC_DIR}katajainen.cc ${ZOPFLI_SRC_DIR}lz77.cc ${ZOPFLI_SRC_DIR}squeeze.cc ${ZOPFLI_SRC_DIR}tree.cc ${ZOPFLI_SRC_DIR}util.cc ${ZOPFLI_SRC_DIR}/zlib_container.cc ${ZOPFLI_SRC_DIR}/zopfli_lib.cc ${ZOPFLI_SRC_DIR}/zopfli.h)
SET_TARGET_PROPERTIES(zopfli_lib PROPERTIES LINKER_LANGUAGE CXX)

# -Wall -Wextra
add_definitions(-g -gstabs+)

add_executable(blobserver blobserver.cpp Config.cpp BlobKey.cpp Blob.cpp BlobIndex.cpp Hash.cpp connection.cpp mime_types.cpp request_handler.cpp server.cpp reply.cpp request_parser.cpp request.hpp header.hpp boost-multipart-form.cpp header.cpp Sync.cpp)
TARGET_LINK_LIBRARIES(blobserver ${BOOST_SYS} ${BOOST_FS} ${BOOST_THR} ${BOOST_PO} ${BOOST_IO} ${BOOST_SE} ${CRYPTOPP} curl pthread json_spirit_lib zopfli_lib m)
