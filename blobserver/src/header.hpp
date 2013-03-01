
#ifndef HTTP_SERVER3_HEADER_HPP
#define HTTP_SERVER3_HEADER_HPP

#include <string>
#include <vector>
#include <utility>

namespace http {
	namespace server3 {

		class header {

			public:

				header();

				explicit header(std::string name, std::string value);

				std::string name();

				void name(std::string name);

				void name(char chr);

				std::string value();

				void value(std::string value);

				void value(char chr) ;

			private:
				std::string name_;
				std::string value_;
				std::vector<std::pair<std::string, std::string> > attributes_;
		};

	}
}

#endif
