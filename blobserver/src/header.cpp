
#include "header.hpp"

#include <string>
#include <vector>
#include <utility>
#include <sstream>

namespace http {
	namespace server3 {


		header::header() { }

		header::header(std::string name, std::string value) : name_(name), value_(value) {
		}

		std::string header::name() const {
			return name_;
		}

		void header::name(std::string name) {
			name_ = name;
		}

		void header::name(char chr) {
			name_.push_back(chr);
		}

		std::string header::value() const {
			return value_;
		}

		void header::value(std::string value) {
			value_ = value;
		}

		void header::value(char chr) {
			value_.push_back(chr);
		}

#if defined ENABLE_DUMP
	std::ostream& header::dump(std::ostream& o) const {
		std::stringstream ss;
		ss << "header{key='" << name() << "' value='" << value() << "'}";
		return o << ss.str();
	}
#endif

	}
}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const http::server3::header& h) {
	return h.dump(o);
}
#endif
