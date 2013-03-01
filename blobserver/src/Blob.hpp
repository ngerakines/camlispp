#ifndef __BLOBSERVER_BLOB_H__
#define __BLOBSERVER_BLOB_H__

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <boost/container/map.hpp>

#include "city.h"

namespace blobserver {

	class Blob {
		public:
			Blob(int base_hash, std::string filePath) : base_hash_(base_hash), filePath_(filePath), size_(0) { }

			std::string filePath() {
				return filePath_;
			}

			int size() {
				return size_;
			}
			void size(int size) {
				size_ = size;
			}

			int base_hash() {
				return base_hash_;
			}

			std::string ref() {
				std::stringstream ss (std::stringstream::in | std::stringstream::out);
				ss << "ch32-" << base_hash_;
				return ss.str();
			}

			bool is_match(std::string hash_type, std::string encoded_hash);

			virtual std::ostream& dump(std::ostream& o) const {
				std::stringstream ss;
				ss << "Blob{ base_hash=" << base_hash_ << " filePath='" << filePath_ << "' size=" << size_ << "}";
				return o << ss.str();
			}

		private:
			uint32 base_hash_;
			std::string filePath_;
			int size_;
			boost::container::map<std::string, std::vector<char> > hashes_;
	};

}

std::ostream& operator<<(std::ostream& o, const blobserver::Blob& b);

#endif /* __BLOBSERVER_BLOB_H__ */
