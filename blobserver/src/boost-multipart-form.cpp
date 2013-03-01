
#include "boost-multipart-form.h"

#include <iostream>
#include <string>
#include <vector>

namespace blobserver {

MultiPartFormData::MultiPartFormData(std::string boundry, std::string input) {
	chunks<std::string::iterator> chunks_parser(boundry);
	std::vector<std::string> chunks_vector;
	bool result = qi::parse(input.begin(), input.end(), chunks_parser, chunks_vector);

	if (!result) {
		return;
	}

	boundry_ = boundry;
	std::vector<std::string>::iterator chunks_iter;

	for (chunks_iter = chunks_vector.begin(); chunks_iter != chunks_vector.end(); ++chunks_iter) {
		std::string chunk_data = *chunks_iter;
		chunk_tokens<std::string::iterator> chunk_parser;
		std::vector<std::string> chunk_vector;
		bool chunk_result = qi::parse(chunk_data.begin(), chunk_data.end(), chunk_parser, chunk_vector);

		if (chunk_result) {
			Part *part = new Part();
			std::vector<std::string>::reverse_iterator chunk_iter;
			bool handled_last = false;

			for (chunk_iter = chunk_vector.rbegin(); chunk_iter != chunk_vector.rend(); ++chunk_iter) {
				if (handled_last) {
					part->add_header(*chunk_iter);

				} else {
					part->payload(*chunk_iter);
				}

				handled_last = true;
			}

			add_part(part);
		}
	}
}

MultiPartFormData::~MultiPartFormData() {
	for (int i = 0; i < (int) parts_.size(); i++) {
		delete parts_[i];
	}
	parts_.clear();
}

Part::~Part() {
	for (int i = 0; i < (int) headers_.size(); i++) {
		delete headers_[i];
	}
	headers_.clear();
}


Header::Header(std::string name, std::map<std::string, std::string> attributes) : name_(name), attributes_(attributes) {
}

boost::optional<std::string> Header::attributeValue(std::string name) {
	std::string lower_name = boost::to_lower_copy(name);
	std::map<std::string, std::string>::iterator iter;

	for (iter = attributes_.begin(); iter != attributes_.end(); ++iter) {
		if (boost::to_lower_copy((*iter).first) == lower_name) {
			return boost::optional<std::string>((*iter).second);
		}
	}

	return boost::optional<std::string>();
}

void Part::add_header(std::string input) {
	unsigned split = input.find(":");

	if (split == std::string::npos) {
		LOG_ERROR("header not added: " << input << std::endl);
		return;
	}

	std::string header_name = input.substr(0, split);
	std::string header_value = input.substr(split + 1);
	header_tokens<std::string::iterator> header_tokens_parser;
	std::map<std::string, std::string> header_tokens_map;
	bool result = qi::parse(header_value.begin(), header_value.end(), header_tokens_parser, header_tokens_map);

	if (result == false) {
		LOG_ERROR("header not added: " << input << std::endl);
		return;
	}

	Header *header = new Header(header_name, header_tokens_map);
	add_header(header);
}

}

std::ostream& operator<<(std::ostream& o, const blobserver::MultiPartFormData& b) { return b.dump(o); }

std::ostream& operator<<(std::ostream& o, const blobserver::Part& b) { return b.dump(o); }

std::ostream& operator<<(std::ostream& o, const blobserver::Header& b) { return b.dump(o); }
