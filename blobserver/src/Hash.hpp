
#ifndef __HASH_H__
#define __HASH_H__

#include <string>
#include <boost/optional.hpp>

#include "config.h"

namespace blobserver {

enum class HashType : int {
	    sha1 = 1,
	    sha256 = 2
	};

	class Sha1 {
		public:
			std::string operator()(const char *s, size_t len) const;
	};

	class Sha256 {
		public:
			std::string operator()(const char *s, size_t len) const;
	};

	boost::optional<HashType> hash_type_for_name(std::string hash_type_name);

	boost::optional<std::string> name_for_hash_type(HashType hash_type);

}

#endif
