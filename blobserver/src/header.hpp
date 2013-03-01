
#ifndef HTTP_SERVER3_HEADER_HPP
#define HTTP_SERVER3_HEADER_HPP

#include <string>
#include <vector>
#include <utility>

namespace http {
	namespace server3 {

		class header {

			public:

				header() { }

				header(std::string name, std::string value) : name_(name), value_(value) {
				}

				std::string name() {
					return name_;
				}

				void name(std::string name) {
					name_ = name;
				}

				void name(char chr) {
					name_.push_back(chr);
				}

				std::string value() {
					return value_;
				}

				void value(std::string value) {
					value_ = value;
				}

				void value(char chr) {
					value_.push_back(chr);
				}

			private:
				std::string name_;
				std::string value_;
				std::vector<std::pair<std::string, std::string> > attributes_;
		};

	}
}

#endif
