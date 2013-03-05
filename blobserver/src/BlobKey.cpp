
#include "BlobKey.hpp"

#include <string>
#include <boost/optional.hpp>

namespace blobserver {

	BlobKey::BlobKey(std::string hash_type, std::string hash_value) : hash_type_(hash_type), hash_value_(hash_value) {
	}

	// NKG: There are two things that I'm not a fan of (a) exceptions and
	// (b) null. I'm using optional here to imply that a blob key could
	// not be made from a given hash.
	boost::optional<BlobKey> create_blob_key(std::string hash) {
		unsigned split_pos = hash.find("-");

		if (split_pos != std::string::npos) {
			std::string hash_type = hash.substr(0, split_pos);
			std::string encoded_hash = hash.substr(split_pos + 1);
			return boost::optional<BlobKey>(BlobKey(hash_type, encoded_hash));
		}

		return boost::optional<BlobKey>();
	}

}
