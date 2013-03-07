#include "BlobIndex.hpp"

#include <assert.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include "logger.hpp"
#include "Hash.hpp"

namespace blobserver {

	BlobIndex::BlobIndex(Config *config) : config_(config) { }

	BlobIndex::~BlobIndex() {
		// NKG: This should be cleaned up.
		std::set<Blob*> blobs;

for (auto & entry : blobs_) {
			blobs.insert(entry.second);
		}

		for (auto iterator = blobs.begin(); iterator != blobs.end(); ++iterator) {
			delete *iterator;
		}

		blobs_.clear();
	}

	// NKG: Still undecided on vector<char> vs char*
	boost::optional<Blob*> BlobIndex::add_blob(boost::optional<std::string> provided_hash, std::vector<char> *bytes) {
		return add_blob(provided_hash, bytes, {HashType::city, HashType::md5, HashType::sha1, HashType::sha256, HashType::murmur3});
	}

	boost::optional<Blob*> BlobIndex::add_blob(boost::optional<std::string> provided_hash, std::vector<char> *bytes, std::vector<HashType> hash_types) {
		boost::mutex::scoped_lock lock(mutex_);
		assert(bytes != NULL);
		assert(bytes->size() > 0);
		auto sbuffer = std::string(bytes->begin(), bytes->end());
		// assert(sbuffer != NULL);
		auto buffer = sbuffer.c_str();
		auto buffer_length = bytes->size();
		std::string hash = CityHash()(buffer, buffer_length);
		std::string path = create_path(hash);
		Blob *b = new Blob(path);
		b->size(bytes->size());
		std::vector<BlobKey> blob_keys;

for (HashType & hash_type : hash_types) {
			switch (hash_type) {
#if defined ENABLE_MD5

			case HashType::md5: {
				std::string hash = MessageDigest5()(buffer, buffer_length);
				BlobKey blob_key("md5", hash);
				blob_keys.push_back(blob_key);
				break;
			}

#endif

			case HashType::sha1: {
				std::string hash = Sha1()(buffer, buffer_length);
				BlobKey blob_key("sha1", hash);
				blob_keys.push_back(blob_key);
				break;
			}

			case HashType::sha256: {
				std::string hash = Sha256()(buffer, buffer_length);
				BlobKey blob_key("sha256", hash);
				blob_keys.push_back(blob_key);
				break;
			}

			case HashType::city: {
				BlobKey blob_key("ch32", hash);
				blob_keys.push_back(blob_key);
				break;
			}

			case HashType::murmur3: {
				std::string hash = Murmur3()(buffer, buffer_length);
				BlobKey blob_key("murmur3", hash);
				blob_keys.push_back(blob_key);
				break;
			}
			}
		}

		if (provided_hash) {
			bool found = false;
			auto provided_hash_blob_key = create_blob_key(*provided_hash);

			if (provided_hash_blob_key) {
for (BlobKey & blob_key : blob_keys) {
					if (blob_key.blobref() == *provided_hash) {
						found = true;
					}
				}
			}

			if (found == false) {
				LOG_ERROR("provided hash mismatch " << *provided_hash << std::endl);
				return boost::optional<Blob*>();
			}
		}

for (BlobKey & blob_key : blob_keys) {
			b->add_hash(blob_key.blobref());
			blobs_[blob_key] = b;
		}

		if (!boost::filesystem::exists(path)) {
			LOG_INFO("Blob does not exist on disk, writing to " << path << std::endl);
			std::ofstream fs(path.c_str(), std::ofstream::binary);
			fs.write(buffer, buffer_length);
			fs.close();
		}

		LOG_INFO("Blob added " << std::endl << DUMP_BLOB(*b) << std::endl);
		return boost::optional<Blob*>(b);
	}

	std::string BlobIndex::create_path(std::string hash) {
		std::string directory = config_->directory();

		if (!boost::ends_with(directory, "/")) {
			directory += "/";
		}

		// NKG: Please don't judge me for this.
		std::string first = hash.substr(0, 3);
		int first_int = atoi(first.c_str());
		std::ostringstream ss;
		ss << std::hex << std::uppercase << std::setfill('0') << first_int;
		directory += ss.str() + "/";
		boost::filesystem::create_directories(directory);
		return directory + hash + ".dat";
	}

	Blob* BlobIndex::get(std::string hash) {
		boost::mutex::scoped_lock lock(mutex_);
		boost::optional<BlobKey> blobKey = create_blob_key(hash);

		if (blobKey) {
			auto found_blob = blobs_.find(*blobKey);

			if (found_blob != blobs_.end()) {
				return found_blob->second;
			}
		}

		return NULL;
	}

	bool BlobIndex::empty() {
		boost::mutex::scoped_lock lock(mutex_);
		return blobs_.empty();
	}

	int BlobIndex::size() {
		boost::mutex::scoped_lock lock(mutex_);
		return (int) blobs_.size();
	}

	void BlobIndex::paginate(std::vector<std::pair<BlobKey, Blob*>> *blobs, boost::optional<std::string> last, int count) {
		std::map<BlobKey, Blob*>::iterator iterator;

		if (last) {
			boost::optional<BlobKey> b = create_blob_key(*last);

			if (b) {
				iterator = blobs_.lower_bound(*b);

			} else {
				iterator = blobs_.begin();
			}

		} else {
			iterator = blobs_.begin();
		}

		for (; iterator != blobs_.end(); ++iterator) {
			blobs->push_back(std::make_pair(iterator->first, iterator->second));

			if (blobs->size() >= count) {
				return;
			}
		}
	}

}
