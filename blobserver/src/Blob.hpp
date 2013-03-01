#ifndef __BLOBSERVER_BLOB_H__
#define __BLOBSERVER_BLOB_H__

#include <string>
#include <vector>
#include <iostream>

#include <boost/container/map.hpp>

#include "config.h"
#include "city.h"

#if defined ENABLE_DUMP
#define DUMP_BLOB(x) x
#else
#define DUMP_BLOB(x) ""
#endif

namespace blobserver {

	class Blob final {
		public:
			explicit Blob(int base_hash, std::string filePath);

			std::string filePath() ;

			int size();
			void size(int size);

			int base_hash();

			std::string ref();

			bool is_match(std::string hash_type, std::string encoded_hash);

#if defined ENABLE_DUMP
			std::ostream& dump(std::ostream& o) const;
#endif
		private:
			uint32 base_hash_;
			std::string filePath_;
			int size_;
			boost::container::map<std::string, std::vector<char> > hashes_;
	};

}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const blobserver::Blob& b);
#endif

#endif /* __BLOBSERVER_BLOB_H__ */
