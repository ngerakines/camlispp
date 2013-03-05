
#include "Hash.hpp"

#include <string>
#include <boost/lexical_cast.hpp>
#include "city.h"
#include <cryptopp/sha.h>
#include <cryptopp/md5.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

namespace blobserver {

	std::string CityHash::operator()(const char *s, size_t len) const {
		uint32 hash = CityHash32(s, len);
		return boost::lexical_cast<std::string>(hash);
	}

#if defined ENABLE_MD5
	std::string MessageDigest5::operator()(const char *s, size_t len) const {
		CryptoPP::Weak::MD5 md5;
		std::string hash = "";
		CryptoPP::StringSource(s, true, new CryptoPP::HashFilter(md5, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
		return hash;
	}
#endif

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

	std::string Murmur3::operator()(const char *s, size_t len) const {
		return "";
	}

}