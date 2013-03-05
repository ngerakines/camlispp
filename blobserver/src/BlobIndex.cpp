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
	Blob* BlobIndex::addBlob(std::vector<char> bytes) {
		boost::mutex::scoped_lock lock(mutex_);
		auto buffer = std::string(bytes.begin(), bytes.end()).c_str();
		auto buffer_length = bytes.size();
		std::string hash = CityHash()(buffer, buffer_length);
		BlobKey blob_key("ch32", hash);
		Blob *b = new Blob(hash);
		b->add_hash("ch32-" + hash);
		std::string full_path = config_->directory() + b ->filePath();

		if (!boost::filesystem::exists(full_path)) {
			LOG_INFO("Blob does not exist on disk, writing to " << full_path << std::endl);
			std::ofstream fs(full_path.c_str(), std::ofstream::binary);
			fs.write(buffer, strlen(buffer));
			fs.close();

		} else {
			// TODO[NKG]: Determine that the file on path matches the provided bytes.
		}

		blobs_[blob_key] = b;
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

}

