#include "BlobIndex.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include "city.h"

#include "logger.hpp"

namespace blobserver {

	BlobIndex::BlobIndex(Config *config) : config_(config) { }

	BlobIndex::~BlobIndex() {
		for (auto &entry : blobs_) {
			delete entry.second;
		}
		blobs_.clear();
	}

	void BlobIndex::addBlob(std::vector<char> bytes) {
		boost::mutex::scoped_lock lock(mutex_);

		// NKG: This could be better, shouldn't have to create a new byte buffer for this.
		auto sbuffer = std::string(bytes.begin(), bytes.end());
		uint32 hash = CityHash32(sbuffer.c_str(), sbuffer.size());

		std::string file_name = boost::lexical_cast<std::string>(hash);

		BlobKey blob_key("ch32", file_name);

		Blob *b = new Blob(hash, file_name);
		std::string full_path = config_->directory() + b ->filePath();

		if (!boost::filesystem::exists(full_path)) {
			LOG_INFO("Blob does not exist on disk, writing to " << full_path << std::endl);
			std::ofstream fs(full_path.c_str(), std::ofstream::binary);
			fs.write(sbuffer.c_str(), sbuffer.size());
			fs.close();
		}

		blobs_[blob_key] = b;
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

