
#include "Blob.hpp"

#include <sstream>

#include <boost/lexical_cast.hpp>

#include "logger.hpp"

namespace blobserver {

	Blob::Blob(int base_hash, std::string filePath) : base_hash_(base_hash), filePath_(filePath), size_(0) {
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

	int Blob::base_hash() {
		return base_hash_;
	}

	std::string Blob::ref() {
		std::stringstream ss (std::stringstream::in | std::stringstream::out);
		ss << "ch32-" << base_hash_;
		return ss.str();
	}

	bool Blob::is_match(std::string hash_type, std::string encoded_hash) {
		if (hash_type == "ch32") {
			LOG_INFO("checking hash of '" << encoded_hash << "' against " << base_hash_ << std::endl);
			uint32 x = boost::lexical_cast<uint32>(encoded_hash);

			if (x == base_hash_) {
				LOG_INFO("match!" << std::endl);
				return true;
			}
		}

		return false;
	}

#if defined ENABLE_DUMP
	std::ostream& Blob::dump(std::ostream& o) const {
		std::stringstream ss;
		ss << "Blob{ base_hash=" << base_hash_ << " filePath='" << filePath_ << "' size=" << size_ << "}";
		return o << ss.str();
	}
#endif

}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const blobserver::Blob& b) {
	return b.dump(o);
}
#endif

