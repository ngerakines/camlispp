
#ifndef HTTP_SERVER3_REQUEST_HPP
#define HTTP_SERVER3_REQUEST_HPP

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include "header.hpp"

namespace qi = boost::spirit::qi;

namespace http {
	namespace server3 {

		// TODO: Rename and/or move this to the request parser header.
		template <typename Iterator>
		struct keys_and_values
		: qi::grammar<Iterator, std::map<std::string, std::string>()> {
		    keys_and_values()
			    : keys_and_values::base_type(query) {
			query =  pair >> *((qi::lit(';') | qi::lit('&')) >> pair);
			pair  =  key >> -('=' >> value);
			key   =  qi::char_("a-zA-Z_") >> *qi::char_("a-zA-Z_0-9");
			value = +qi::char_("/:_@a-zA-Z0-9.,+*!=-");
		}
		qi::rule<Iterator, std::map<std::string, std::string>()> query;
		qi::rule<Iterator, std::pair<std::string, std::string>()> pair;
		qi::rule<Iterator, std::string()> key, value;
		};

		class request {
			public:

				std::string	method;
				std::string	uri;
				int	http_version_major;
				int	http_version_minor;
				std::vector<header>	headers;
				std::string	content;
				std::string query_string;
				boost::posix_time::ptime tstamp;

				void clear() {
					method.clear();
					uri.clear();
					http_version_major = 0;
					http_version_minor = 0;
					headers.clear();
					content.clear();
				}

				std::string get_header(std::string name) {
					std::vector<header>::iterator hi;

					for (hi = headers.begin(); hi != headers.end(); hi++) {
						if ((*hi).name() == name) {
							return (*hi).value();
						}
					}

					return "";
				}

				bool is_http11() const {
					if ((http_version_major > 1) ||
					        ((http_version_major == 1) && (http_version_minor > 0)))
					{ return true; }

					return false;
				}

				bool want_keepalive() {
					bool rc = is_http11();
					std::string cxn_hdr =
					    boost::to_lower_copy(get_header("connection"));

					if (cxn_hdr == "close")
					{ rc = false; }

					else if (cxn_hdr == "keep-alive")
					{ rc = true; }

					return rc;
				}
		};

	}
}

#endif
