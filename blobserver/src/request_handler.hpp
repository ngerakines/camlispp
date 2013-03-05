#ifndef HTTP_SERVER3_REQUEST_HANDLER_HPP
#define HTTP_SERVER3_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "header.hpp"
#include "boost-multipart-form.h"

#include "BlobIndex.hpp"

namespace http {

	namespace server3 {

		struct reply;
		struct request;

		enum state {
		    s_uninitialized = 1,
		    s_start,
		    s_start_boundary,
		    s_header_field_start,
		    s_header_field,
		    s_headers_almost_done,
		    s_header_value_start,
		    s_header_value,
		    s_header_value_almost_done,
		    s_part_data_start,
		    s_part_data,
		    s_part_data_almost_boundary,
		    s_part_data_boundary,
		    s_part_data_almost_end,
		    s_part_data_end,
		    s_part_data_final_hyphen,
		    s_end
		};

		class request_handler : private boost::noncopyable {
			public:
				/// Construct with a directory containing files to be served.
				explicit request_handler(blobserver::BlobIndex *bi, const std::string& doc_root);

				/// Handle a request and produce a reply.
				void handle_request(const request& req, reply& rep);

				blobserver::Header* parse_boundry_header(std::string header_value);
			private:
				blobserver::BlobIndex *bi_;

				/// The directory containing the files to be served.
				std::string doc_root_;

				/// Perform URL-decoding on a string. Returns false if the encoding was
				/// invalid.
				static bool url_decode(const std::string& in, std::string& out);

				void handle_get(std::string request_path, const request& req, reply& rep);

				void handle_put(std::string request_path, const request& req, reply& rep);

				void handle_stat(std::string request_path, const request& req, reply& rep);

				void handle_check(std::string request_path, const request& req, reply& rep);

				void handle_enumerate(std::string request_path, const request& req, reply& rep);

				void decode_query_string_blobs(std::vector<std::string>* blobs, std::string request_path);

				std::string get_header(std::vector<header> headers, std::string name);

		};

	}
}

#endif
