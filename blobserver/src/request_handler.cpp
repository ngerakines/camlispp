#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "json_spirit_reader.h"
#include "json_spirit_writer.h"
#include "json_spirit_utils.h"

#include "deflate.h"
#include "gzip_container.h"
#include "zlib_container.h"
#include "zopfli.h"

#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "Blob.hpp"
#include "logger.hpp"
#include "header.hpp"

using namespace blobserver;

namespace http {
	namespace server3 {

		request_handler::request_handler(blobserver::Config *c, blobserver::BlobIndex *bi) : c_(c), bi_(bi) { }

		void request_handler::handle_request(const request& req, reply& rep) {
			LOG_INFO("handle_request called" << std::endl);
			// Decode url to path.
			std::string request_path;

			if (!url_decode(req.uri, request_path)) {
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

			LOG_INFO(" - request_path = " << request_path << std::endl);
			LOG_INFO(" - request method = " << req.method << std::endl);
			LOG_INFO(" - request headers:"<< std::endl);
			for (auto &h : req.headers) {
				LOG_INFO("   - " << h.name() << "=" << h.value() << std::endl);
			}

			// Request path must be absolute and not contain "..".
			if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos) {
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

#if defined ENABLE_STATIC

			if (boost::starts_with(request_path, "/static/")) {
				handle_static(request_path, req, rep);
				return;
			}

#endif

			if (boost::starts_with(request_path, "/stat")) {
				handle_stat(request_path, req, rep);
				return;
			}

			if (boost::starts_with(request_path, "/enumerate-blobs") && req.method.compare("GET") == 0) {
				handle_enumerate(request_path, req, rep);
				return;
			}

			if (req.method.compare("HEAD") == 0) {
				handle_check(request_path, req, rep);
				return;
			}

			if (req.method.compare("GET") == 0) {
				handle_get(request_path, req, rep);
				return;
			}

			if (req.method.compare("PUT") == 0 || req.method.compare("POST") == 0) {
				handle_put(request_path, req, rep);
				return;
			}

			rep = reply::stock_reply(reply::bad_request);
		}

#if defined ENABLE_STATIC
		void request_handler::handle_static(std::string request_path, const request& req, reply& rep) {
			// If path ends in slash (i.e. is a directory) then add "index.html".
			if (request_path[request_path.size() - 1] == '/') {
				request_path += "index.html";
			}

			// Determine the file extension.
			std::size_t last_slash_pos = request_path.find_last_of("/");
			std::size_t last_dot_pos = request_path.find_last_of(".");
			std::string extension;

			if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
				extension = request_path.substr(last_dot_pos + 1);
			}

			// Open the file to send back.
			std::string full_path = c_->static_directory() + request_path;
			std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
			LOG_INFO("looking for " << full_path << std::endl);

			if (!is) {
				rep = reply::stock_reply(reply::not_found);
				return;
			}

			// Fill out the reply to be sent to the client.
			rep.status = reply::ok;
			char buf[512];

			while (is.read(buf, sizeof(buf)).gcount() > 0) {
				rep.content.append(buf, is.gcount());
			}

			rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(rep.content.size())));
			rep.headers.push_back(header("Content-Type", mime_types::extension_to_type(extension)));
		}

#endif

		void request_handler::handle_check(std::string request_path, const request& req, reply& rep) {
			std::string blob_ref = request_path.substr(1);
			Blob *foundBlob = bi_->get(blob_ref);

			if (foundBlob == NULL) {
				LOG_ERROR("blob not found: " << blob_ref);
				rep = reply::stock_reply(reply::not_found);
				return;
			}

			/* std::string full_path = foundBlob->filePath();

			if (!boost::filesystem::exists(full_path)) {
				LOG_INFO("file not found: " << full_path);
				rep = reply::stock_reply(reply::not_found);
				return;
			} */

			rep.status = reply::ok;
			rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(foundBlob->size())));
			rep.headers.push_back(header("Content-Type", "application/octet-stream"));
		}

		void request_handler::handle_get(std::string request_path, const request& req, reply& rep) {
			std::string blob_ref = request_path.substr(1);
			Blob *foundBlob = bi_->get(blob_ref);

			if (foundBlob == NULL) {
				LOG_INFO("blob not found: " << blob_ref);
				rep = reply::stock_reply(reply::not_found);
				return;
			}

#if defined ENABLE_DUMP
			LOG_INFO(DUMP_BLOB(*foundBlob) << std::endl);
#endif
			std::string full_path = foundBlob->filePath();
			LOG_INFO("loading file from path " << full_path << std::endl);
			std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);

			if (!is) {
				LOG_INFO("file not found: " << full_path);
				rep = reply::stock_reply(reply::not_found);
				return;
			}

			bool compress_output = false;
			ZopfliFormat output_type = ZOPFLI_FORMAT_GZIP;
			std::string accept_encoding = get_header(req.headers, "Accept-Encoding");
			if (accept_encoding != "") {
				LOG_INFO("accept_encoding" << accept_encoding << std::endl);
				if (boost::starts_with(accept_encoding, "gzip")) {
					compress_output = true;
				} else if (boost::starts_with(accept_encoding, "deflate")) {
					compress_output = true;
					output_type = ZOPFLI_FORMAT_DEFLATE;
				} else if (boost::starts_with(accept_encoding, "zlib")){
					compress_output = true;
					output_type = ZOPFLI_FORMAT_ZLIB;
				}
			}

			rep.status = reply::ok;
			char buf[512];

			// NKG: Instead of reading chunks into a file, we should be writing them directly to the output stream.
			while (is.read(buf, sizeof(buf)).gcount() > 0) {
				rep.content.append(buf, is.gcount());
			}

			// NKG: I'd rather compress the chunks read from file instead of this current two step process.
			if (compress_output) {
				LOG_INFO("compressing" << std::endl);
				const unsigned char* in = reinterpret_cast<const unsigned char *>(rep.content.c_str());
				unsigned char* out = 0;
				size_t insize = rep.content.length();
				LOG_INFO("old length " << insize << std::endl);
				size_t outsize = 0;
				ZopfliOptions options;
				ZopfliInitOptions(&options);
				ZopfliCompress(reinterpret_cast<const ZopfliOptions*>(&options), output_type, in, insize, &out, &outsize);
				LOG_INFO("new length " << outsize << std::endl);
				rep.content = std::string(reinterpret_cast<const char*>(out), outsize);
			}

			rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(rep.content.size())));
			rep.headers.push_back(header("Content-Type", "application/octet-stream"));
			if (compress_output) {
				if (output_type == ZOPFLI_FORMAT_GZIP) {
					rep.headers.push_back(header("Content-Encoding", "gzip"));
				} else if (output_type == ZOPFLI_FORMAT_ZLIB) {
					rep.headers.push_back(header("Content-Encoding", "zlib"));
				} else if (output_type == ZOPFLI_FORMAT_DEFLATE) {
					rep.headers.push_back(header("Content-Encoding", "deflate"));
				}
			}
		}

		std::string request_handler::get_header(std::vector<header> headers, std::string name) {
			std::vector<header>::iterator hi;
			std::string lower_name = boost::to_lower_copy(name);

			for (hi = headers.begin(); hi != headers.end(); hi++) {
				if (boost::to_lower_copy((*hi).name()) == lower_name) {
					return (*hi).value();
				}
			}

			return "";
		}

		void request_handler::handle_enumerate(std::string request_path, const request& req, reply& rep) {
			std::vector<std::pair<BlobKey, Blob*>> blobs;
			boost::optional<std::string> after = decode_query_string_after(request_path);
			bi_->paginate(&blobs, after, 26);
			json_spirit::Object result;
			json_spirit::Array stat;

			int count = 0;
			for (auto & pair : blobs) {
				if (count < 25) {
					json_spirit::Object blob_value;
					blob_value.push_back(json_spirit::Pair("blobRef", pair.first.blobref()));
					blob_value.push_back(json_spirit::Pair("size", pair.second->size()));
					stat.push_back(blob_value);
				}
				count++;
			}

			result.push_back(json_spirit::Pair("blobs", stat));

			if (blobs.size() > 25) {
				auto last = blobs.back();
				result.push_back(json_spirit::Pair("continueAfter", last.first.blobref()));
			}

			result.push_back(json_spirit::Pair("canLongPoll", "false"));
			rep.status = reply::ok;
			rep.content = json_spirit::write(result);
			rep.content += "\r\n";
			rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(rep.content.size())));
			rep.headers.push_back(header("Content-Type", "application/json"));
		}

		void request_handler::handle_put(std::string request_path, const request& req, reply& rep) {
			LOG_INFO("handle_put called" << std::endl);
			std::string body = req.content;
			LOG_INFO("body: " << std::endl << body << std::endl);
			std::string content_type = get_header(req.headers, "Content-Type");

			if (content_type == "") {
				rep = reply::stock_reply(reply::bad_request);
				return;
			}

			blobserver::Header *header = parse_boundry_header(content_type);
			LOG_INFO(*(header) << std::endl);
			boost::optional<std::string> boundary = header->attributeValue("boundary");

			if (boundary) {
				LOG_INFO("boundary = '" << *boundary << "'" << std::endl);
				LOG_INFO("body" << std::endl << "******" << std::endl << body << "******" << std::endl);
				MultiPartFormData mpfd(*boundary, body);
				LOG_INFO(mpfd << std::endl);
				BOOST_FOREACH(Part * part, mpfd.parts()) {
					std::vector<char> payload = part->payload();

					if (payload.size() > 0) {
						LOG_INFO("payload is greater than 0" << std::endl);
						bi_->add_blob(boost::optional<std::string>(), &payload);
					}
				}
			}

			delete header;
			rep.status = reply::ok;
		}

		blobserver::Header* request_handler::parse_boundry_header(std::string header_value) {
			header_tokens<std::string::iterator> header_tokens_parser;
			std::map<std::string, std::string> header_tokens_map;
			bool result = qi::parse(header_value.begin(), header_value.end(), header_tokens_parser, header_tokens_map);

			if (result == false) {
				LOG_ERROR("header not added: " << header_value << std::endl);
				header_tokens_map.clear();
				// return boost::scoped_ptr<Header>(new Header("Content-Type", std::map<std::string, std::string>()));
			}

			return new blobserver::Header("Content-Type", header_tokens_map);
		}

		void request_handler::handle_stat(std::string request_path, const request& req, reply& rep) {
			LOG_INFO("handle_stat called" << std::endl);

			std::vector<std::string> blobs;

			if (req.method == "GET") {
				unsigned foundCamliVersion = request_path.find("camliversion=1");

				if (foundCamliVersion == std::string::npos) {
					rep = reply::stock_reply(reply::bad_request);
					return;
				}

				decode_query_string_blobs(&blobs, request_path);
			} else if (req.method == "POST") {
				decode_query_string_blobs(&blobs, req.content);
			}

			json_spirit::Object result;
			json_spirit::Array stat;
			BOOST_FOREACH(std::string blob, blobs) {
				std::cout << blob << std::endl;
				Blob *foundBlob = bi_->get(blob);
				LOG_INFO("blob exists? " << (foundBlob != NULL) << std::endl);
				json_spirit::Object blob_value;
				blob_value.push_back(json_spirit::Pair("blobRef", blob));
				blob_value.push_back(json_spirit::Pair("size", foundBlob->size()));
				stat.push_back(blob_value);
			}
			result.push_back(json_spirit::Pair("stat", stat));
			result.push_back(json_spirit::Pair("maxUploadSize", 1048576));
			result.push_back(json_spirit::Pair("uploadUrl", "/upload"));
			result.push_back(json_spirit::Pair("uploadUrlExpirationSeconds", 7200));
			result.push_back(json_spirit::Pair("canLongPoll", false));
			rep.status = reply::ok;
			rep.content = json_spirit::write(result);
			rep.content += "\r\n";
			rep.headers.push_back(header("Content-Length", boost::lexical_cast<std::string>(rep.content.size())));
			rep.headers.push_back(header("Content-Type", "application/json"));
		}

		void request_handler::decode_query_string_blobs(std::vector<std::string>* blobs, std::string input) {
			unsigned found = input.find("?");

			if (found != std::string::npos) {
				input = input.substr(found + 1);
			}
				std::string::iterator begin = input.begin();
				std::string::iterator end = input.end();
				keys_and_values<std::string::iterator> p;
				std::map<std::string, std::string> m;
				bool result = qi::parse(begin, end, p, m);

				if (result) {
					std::map<std::string, std::string>::iterator iter;

					for (iter = m.begin(); iter != m.end(); ++iter) {
						LOG_INFO(iter->first << " = " << iter->second << std::endl);

						if (boost::starts_with(iter->first, "blob")) {
							blobs->push_back(iter->second);
						}
					}
				}
		}

		boost::optional<std::string> request_handler::decode_query_string_after(std::string request_path) {
			unsigned found = request_path.find("?");

			if (found != std::string::npos) {
				std::string query = request_path.substr(found + 1);
				std::string input(query);
				std::string::iterator begin = input.begin();
				std::string::iterator end = input.end();
				keys_and_values<std::string::iterator> p;
				std::map<std::string, std::string> m;
				bool result = qi::parse(begin, end, p, m);

				if (result) {
					std::map<std::string, std::string>::iterator iter;

					for (iter = m.begin(); iter != m.end(); ++iter) {
						LOG_INFO(iter->first << " = " << iter->second << std::endl);

						if (boost::starts_with(iter->first, "after")) {
							return boost::optional<std::string>(iter->second);
						}
					}
				}
			}
			return boost::optional<std::string>();
		}

		bool request_handler::url_decode(const std::string& in, std::string& out) {
			out.clear();
			out.reserve(in.size());

			for (std::size_t i = 0; i < in.size(); ++i) {
				if (in[i] == '%') {
					if (i + 3 <= in.size()) {
						int value = 0;
						std::istringstream is(in.substr(i + 1, 2));

						if (is >> std::hex >> value) {
							out += static_cast<char>(value);
							i += 2;

						} else {
							return false;
						}

					} else {
						return false;
					}

				} else if (in[i] == '+') {
					out += ' ';

				} else {
					out += in[i];
				}
			}

			return true;
		}

	}
}
