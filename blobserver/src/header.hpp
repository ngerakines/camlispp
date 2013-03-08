
#ifndef HTTP_SERVER3_HEADER_HPP
#define HTTP_SERVER3_HEADER_HPP

#include "config.h"

#include <string>
#include <vector>
#include <utility>
#if defined ENABLE_DUMP
#include <iostream>
#endif

namespace http {
	namespace server3 {

		class header {

			public:

				explicit header();

				explicit header(std::string name, std::string value);

				std::string name() const;

				void name(std::string name);

				void name(char chr);

				std::string value() const;

				void value(std::string value);

				void value(char chr);

#if defined ENABLE_DUMP
				std::ostream& dump(std::ostream& o) const;
#endif

			private:
				std::string name_;
				std::string value_;
				std::vector<std::pair<std::string, std::string> > attributes_;
		};

	}
}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const http::server3::header& h);
#endif

#endif
