#include <string>

#include "BlobIndex.hpp"

#include <fstream>
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include "city.h"

namespace blobserver {

	BlobIndex::BlobIndex(Config *config) : config_(config) { }

	BlobIndex::~BlobIndex() {
		for (int i = 0; i < (int) blobs_.size(); i++) {
			delete blobs_[i];
		}
		blobs_.clear();
	}

	void BlobIndex::addBlob(std::vector<char> bytes) {
		boost::mutex::scoped_lock lock(mutex_);
		std::string sbuffer = std::string(bytes.begin(), bytes.end());
		uint32 hash = CityHash32(sbuffer.c_str(), sbuffer.size());
		LOG_INFO("hash " << hash << std::endl);
		std::string file_name = boost::lexical_cast<std::string>(hash);
		Blob *b = new Blob(hash, file_name);
		std::string full_path = config_->directory() + b ->filePath();

		LOG_INFO("Checking path: " << full_path << std::endl);
		if (!boost::filesystem::exists(full_path)) {
			LOG_INFO("Blob does not exist on disk, writing to " << full_path << std::endl);
			std::ofstream fs(full_path.c_str(), std::ofstream::binary);
			fs.write(sbuffer.c_str(), sbuffer.size());
			fs.close();
		}

		blobs_.push_back(b);
	}

	Blob* BlobIndex::get(std::string hash) {
		boost::mutex::scoped_lock lock(mutex_);
		unsigned split_pos = hash.find("-");
		LOG_INFO("split position " << split_pos << std::endl);

		if (split_pos != std::string::npos) {
			std::string hash_type = hash.substr(0, split_pos);
			std::string encoded_hash = hash.substr(split_pos + 1);
			LOG_INFO("hash type = " << hash_type << " ; hash = " << encoded_hash << std::endl);
			BOOST_FOREACH(Blob * blob, blobs_) {
				if (blob->is_match(hash_type, encoded_hash)) {
					LOG_INFO(*blob << std::endl);
					return blob;
				} else {
				}
			}
		}

		return NULL;
	}

}

