
#include "boost-multipart-form.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/range/adaptors.hpp>
#include <boost/algorithm/string/erase.hpp>

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

				for (std::string &chunk : boost::adaptors::reverse(chunk_vector)) {
					if (handled_last) {
						part->add_header(chunk);
					} else {
						boost::erase_last(chunk, "\r\n");
						part->payload(chunk);
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

	std::string MultiPartFormData::boundry() {
		return boundry_;
	}

	void MultiPartFormData::boundry(std::string boundry) {
		boundry_ = boundry;
	}

	std::vector<Part*> MultiPartFormData::parts() const {
		return parts_;
	}

	void MultiPartFormData::add_part(Part* part) {
		parts_.push_back(part);
	}

#if defined ENABLE_DUMP
	std::ostream& MultiPartFormData::dump(std::ostream& o) const {
		std::stringstream ss;
		ss << "MultiPartFormData{ boundry='" << boundry_ << "'";
		std::vector<Part*> prts = parts();
		std::vector<Part*>::iterator iter;
		ss << " parts=[" << std::endl;

		for (iter = prts.begin(); iter != prts.end(); ++iter) {
			ss << " " << *(*iter) << std::endl;
		}

		ss << "]" << std::endl;
		ss << "}";
		return o << ss.str();
	}
#endif

	Part::~Part() {
		for (int i = 0; i < (int) headers_.size(); i++) {
			delete headers_[i];
		}

		headers_.clear();
	}

	std::vector<Header*> Part::headers() {
		return headers_;
	}

	std::vector<char> Part::payload() {
		return payload_;
	}

	void Part::payload(std::string payload) {
		payload_.assign(payload.begin(), payload.end());
	}

	void Part::payload(std::vector<char> payload) {
		payload_ = payload;
	}

	void Part::add_header(Header* header) {
		headers_.push_back(header);
	}

	std::vector<Header*> Part::headers() const {
		return headers_;
	}

#if defined ENABLE_DUMP
	std::ostream& Part::dump(std::ostream& o) const {
		std::stringstream ss;
		ss << "Part{";
		std::string payload(payload_.begin(), payload_.end());
		ss << " payload='" << payload << "'";
		std::vector<Header*> hdrs = headers();
		ss << " headers=[";
		std::vector<Header*>::iterator iter;

		for (iter = hdrs.begin(); iter != hdrs.end(); ++iter) {
			if (iter != hdrs.begin()) {
				ss << ", ";
			}

			ss << *(*iter);
		}

		ss << "]";
		ss << "}";
		return o << ss.str();
	}
#endif

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

	Header::Header(std::string name, std::map<std::string, std::string> attributes) : name_(name), attributes_(attributes) {
	}

	std::string Header::name() {
		return name_;
	}

	std::map<std::string, std::string> Header::attributes() const {
		return attributes_;
	}

#if defined ENABLE_DUMP
	std::ostream& Header::dump(std::ostream& o) const {
		std::stringstream ss;
		ss << "Header{";
		ss << " name='" << name_ << "'";
		std::map<std::string, std::string> attrs = attributes();
		std::map<std::string, std::string>::iterator iter;
		ss << " attributes=[";

		for (iter = attrs.begin(); iter != attrs.end(); ++iter) {
			if (iter != attrs.begin()) {
				ss << ", ";
			}

			ss << iter->first;

			if (iter->second != "") {
				ss << "=" << iter->second;
			}
		}

		ss << "]";
		ss << "}";
		return o << ss.str();
	}
#endif

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

}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const blobserver::MultiPartFormData& b) { return b.dump(o); }

std::ostream& operator<<(std::ostream& o, const blobserver::Part& b) { return b.dump(o); }

std::ostream& operator<<(std::ostream& o, const blobserver::Header& b) { return b.dump(o); }
#endif
