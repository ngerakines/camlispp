
#ifndef __MULTIPART_FORM_H__
#define __MULTIPART_FORM_H__

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/optional.hpp>

#include "logger.hpp"

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

namespace blobserver {

class Header;
class Part;
class MultiPartFormData;

}

std::ostream& operator<<(std::ostream& o, const blobserver::MultiPartFormData& b);
std::ostream& operator<<(std::ostream& o, const blobserver::Header& b);
std::ostream& operator<<(std::ostream& o, const blobserver::Part& b);

namespace blobserver {

template <typename Iterator>
struct header_tokens : qi::grammar<Iterator, std::map<std::string, std::string>()> {

    header_tokens() : header_tokens::base_type(query) {
	query =  pair >> *(qi::lit(';') >> *(" ") >> pair) >> -(boost::spirit::eol);
	pair =  key >> -(qi::lit('=') >> value);
	key = +(ascii::char_ - qi::char_("=;"));
	value = (qi::lexeme['"' >> +(ascii::char_ - '"') >> '"'] | +(ascii::char_));
}

qi::rule<Iterator, std::map<std::string, std::string>()> query;
qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
qi::rule<Iterator, std::string()> key, value;
};

template <typename Iterator>
struct chunks : qi::grammar<Iterator, std::vector<std::string>()> {

    chunks(std::string boundry) : chunks::base_type(content) {
	using qi::lit;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using ascii::char_;
	using ascii::string;
	using namespace qi::labels;
	using phoenix::construct;
	using phoenix::val;
	content = chunk >> *chunk >> end_boundry;
	chunk = chunk_boundry >> value;
	chunk_boundry = qi::lit("--") >> boundry >> boost::spirit::eol;
	end_boundry = qi::lit("--") >> boundry >> qi::lit("--") >> -boost::spirit::eol;
	value = +(ascii::char_ - end_boundry - chunk_boundry);
}

qi::rule<Iterator, std::vector<std::string>()> content;
qi::rule<Iterator, std::string()> value, chunk, chunk_boundry, end_boundry;
};

template <typename Iterator>
struct chunk_tokens : qi::grammar<Iterator, std::vector<std::string>()> {

    chunk_tokens() : chunk_tokens::base_type(content) {
	using qi::lit;
	using qi::lexeme;
	using qi::on_error;
	using qi::fail;
	using ascii::char_;
	using ascii::string;
	using namespace qi::labels;
	using phoenix::construct;
	using phoenix::val;
	content = *header >> boost::spirit::eol >> *body;
	token = header | body;
	header = +(ascii::char_ - boost::spirit::eol) >> boost::spirit::eol;
	body = +(ascii::char_ | boost::spirit::eol);
}

qi::rule<Iterator, std::vector<std::string>()> content;
qi::rule<Iterator, std::string()> token, header, body;
};


class Header {

	public:

		Header() { }

		Header(std::string name, std::map<std::string, std::string> attributes);

		std::string name() {
			return name_;
		}

		std::map<std::string, std::string> attributes() const {
			return attributes_;
		}

		boost::optional<std::string> attributeValue(std::string name);

		virtual std::ostream& dump(std::ostream& o) const {
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

	private:
		std::string name_;
		std::map<std::string, std::string> attributes_;
};

class Part {

	public:
		~Part();

		std::vector<Header*> headers() {
			return headers_;
		}

		std::vector<char> payload() {
			return payload_;
		}

		void payload(std::string payload) {
			payload_.assign(payload.begin(), payload.end());
		}

		void payload(std::vector<char> payload) {
			payload_ = payload;
		}

		void add_header(Header* header) {
			headers_.push_back(header);
		}

		std::vector<Header*> headers() const {
			return headers_;
		}

		void add_header(std::string input);

		virtual std::ostream& dump(std::ostream& o) const {
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

	private:
		std::vector<Header*> headers_;
		std::vector<char> payload_;

};

class MultiPartFormData {

	public:

		MultiPartFormData(std::string boundry, std::string input);
		~MultiPartFormData();

		std::string boundry() {
			return boundry_;
		}

		void boundry(std::string boundry) {
			boundry_ = boundry;
		}

		std::vector<Part*> parts() const {
			return parts_;
		}

		void add_part(Part* part) {
			parts_.push_back(part);
		}

		virtual std::ostream& dump(std::ostream& o) const {

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

	private:
		std::string boundry_;
		std::vector<Part*> parts_;
};

}

#endif
