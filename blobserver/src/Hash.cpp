
#include "Hash.hpp"

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

namespace blobserver {

	std::string Sha1::operator()(const char *s, size_t len) const {
		CryptoPP::SHA1 sha1;
		std::string hash = "";
		CryptoPP::StringSource(s, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
		return hash;
	}

	std::string Sha256::operator()(const char *s, size_t len) const {
		CryptoPP::SHA256 sha256;
		std::string hash = "";
		CryptoPP::StringSource(s, true, new CryptoPP::HashFilter(sha256, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
		return hash;
	}

	boost::optional<HashType> hash_type_for_name(std::string hash_type_name) {
		return boost::optional<HashType>();
	}

	boost::optional<std::string> name_for_hash_type(HashType hash_type) {
		return boost::optional<std::string>();
	}

}
