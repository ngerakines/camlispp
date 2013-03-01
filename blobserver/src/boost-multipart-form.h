
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

#include "config.h"
#include "logger.hpp"

#if defined ENABLE_DUMP
#define DUMP_HEADER(x) x
#define DUMP_PART(x) x
#define DUMP_MPFD(x) x
#else
#define DUMP_HEADER(x) ""
#define DUMP_PART(x) ""
#define DUMP_MPFD(x) ""
#endif

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

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
			Header(std::string name, std::map<std::string, std::string> attributes);

			std::string name();

			std::map<std::string, std::string> attributes() const;

			boost::optional<std::string> attributeValue(std::string name);

#if defined ENABLE_DUMP
			std::ostream& dump(std::ostream& o) const;
#endif

		private:
			std::string name_;
			std::map<std::string, std::string> attributes_;
	};

	class Part {

		public:
			~Part();

			std::vector<Header*> headers();

			std::vector<char> payload();

			void payload(std::string payload);

			void payload(std::vector<char> payload);

			void add_header(Header* header);

			std::vector<Header*> headers() const;

			void add_header(std::string input);

#if defined ENABLE_DUMP
			std::ostream& dump(std::ostream& o) const;
#endif

		private:
			std::vector<Header*> headers_;
			std::vector<char> payload_;

	};

	class MultiPartFormData final {

		public:

			MultiPartFormData(std::string boundry, std::string input);
			~MultiPartFormData();

			std::string boundry();

			void boundry(std::string boundry);

			std::vector<Part*> parts() const;

			void add_part(Part* part);

#if defined ENABLE_DUMP
			std::ostream& dump(std::ostream& o) const;
#endif

		private:
			std::string boundry_;
			std::vector<Part*> parts_;
	};

}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const blobserver::MultiPartFormData& b);
std::ostream& operator<<(std::ostream& o, const blobserver::Header& b);
std::ostream& operator<<(std::ostream& o, const blobserver::Part& b);
#endif

#endif
