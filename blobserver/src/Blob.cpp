
#include "Blob.hpp"

#include <boost/lexical_cast.hpp>

#include "logger.hpp"

namespace blobserver {

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

}

std::ostream& operator<<(std::ostream& o, const blobserver::Blob& b) { return b.dump(o); }

