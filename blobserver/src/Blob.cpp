
#include "Blob.hpp"

#include <sstream>
#include <string>
#include <set>
#include <boost/lexical_cast.hpp>

#include "logger.hpp"

namespace blobserver {

	Blob::Blob(std::string filePath) : filePath_(filePath), size_(0) { }

	Blob::~Blob() {
		hashes_.clear();
	}

	std::string Blob::filePath() {
		return filePath_;
	}

	int Blob::size() {
		return size_;
	}
	void Blob::size(int size) {
		size_ = size;
	}

	void Blob::add_hash(std::string hash) {
		hashes_.insert(hash);
	}

	sset Blob::hashes() {
		return hashes_;
	}

	bool Blob::is_match(std::string hash) {
		auto it = hashes_.find(hash);

		if (it == hashes_.end()) {
			return false;
		}

		return true;
	}

#if defined ENABLE_DUMP
	std::ostream& Blob::dump(std::ostream& o) const {
		std::stringstream ss;
		ss << "Blob{filePath='" << filePath_ << "' size=" << size_ ;
		ss << " hashes=[";
		bool first = true;

for (auto & hash : hashes_) {
			if (!first) {
				ss << ", ";
			}

			ss << hash;
			first = false;
		}

		ss << "]}";
		return o << ss.str();
	}
#endif

}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const blobserver::Blob& b) {
	return b.dump(o);
}
#endif

