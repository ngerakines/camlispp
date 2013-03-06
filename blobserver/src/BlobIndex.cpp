#include "BlobIndex.hpp"

#include <string>
#include <fstream>
#include <iostream>
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
for (auto & entry : blobs_) {
			delete entry.second;
		}

		blobs_.clear();
	}

	// NKG: Still undecided on vector<char> vs char*
	Blob* BlobIndex::add_blob(std::vector<char> *bytes) {
		return add_blob(bytes, {HashType::city, HashType::md5, HashType::sha1, HashType::sha256, HashType::murmur3});
	}

	Blob* BlobIndex::add_blob(std::vector<char> *bytes, std::vector<HashType> hash_types) {
		boost::mutex::scoped_lock lock(mutex_);
		auto buffer = std::string(bytes->begin(), bytes->end()).c_str();
		auto buffer_length = bytes->size();
		std::string hash = CityHash()(buffer, buffer_length);
		Blob *b = new Blob(hash);
		std::string full_path = config_->directory() + b ->filePath();

		if (!boost::filesystem::exists(full_path)) {
			LOG_INFO("Blob does not exist on disk, writing to " << full_path << std::endl);
			std::ofstream fs(full_path.c_str(), std::ofstream::binary);
			fs.write(buffer, strlen(buffer));
			fs.close();
		}

		for (HashType & hash_type : hash_types) {
			switch (hash_type) {
#if defined ENABLE_MD5

			case HashType::md5: {
				std::string hash = MessageDigest5()(buffer, buffer_length);
				BlobKey blob_key("md5", hash);
				blobs_[blob_key] = b;
				b->add_hash("md5-" + hash);
				break;
			}

#endif

			case HashType::sha1: {
				std::string hash = Sha1()(buffer, buffer_length);
				BlobKey blob_key("sha1", hash);
				blobs_[blob_key] = b;
				b->add_hash("sha1-" + hash);
				break;
			}

			case HashType::sha256: {
				std::string hash = Sha256()(buffer, buffer_length);
				BlobKey blob_key("sha256", hash);
				blobs_[blob_key] = b;
				b->add_hash("sha256-" + hash);
				break;
			}

			case HashType::city: {
				BlobKey blob_key("ch32", hash);
				blobs_[blob_key] = b;
				b->add_hash("ch32-" + hash);
				break;
			}

			case HashType::murmur3: {
				std::string hash = Murmur3()(buffer, buffer_length);
				BlobKey blob_key("murmur3", hash);
				blobs_[blob_key] = b;
				b->add_hash("murmur3-" + hash);
				break;
			}
			}
		}

		LOG_INFO("Blob added " << std::endl << DUMP_BLOB(*b) << std::endl);
		return b;
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

	void BlobIndex::paginate(std::vector<std::pair<BlobKey, Blob*>> *blobs) {
		paginate(blobs, boost::optional<std::string>(), 25);
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
