#ifndef __BLOBSERVER_BLOB_H__
#define __BLOBSERVER_BLOB_H__

#include <cstring>
#include <string>
#include <vector>
#include <set>
#if defined ENABLE_DUMP
#include <iostream>
#endif

#include "config.h"
#include "city.h"

#if defined ENABLE_DUMP
#define DUMP_BLOB(x) x
#else
#define DUMP_BLOB(x) ""
#endif

namespace blobserver {

	struct CaseSensitiveCompare {
		bool operator() (const std::string& a, const std::string& b) const {
			return strcmp(a.c_str(), b.c_str()) < 0;
		}
	};

	class Blob final {
		public:
			explicit Blob(std::string filePath);

			std::string filePath();

			int size();
			void size(int size);

			void add_hash(std::string hash);
			// std::set<std::string> hashes();

			bool is_match(std::string hash);

#if defined ENABLE_DUMP
			std::ostream& dump(std::ostream& o) const;
#endif
		private:
			std::string filePath_;
			int size_;
			std::set<std::string, CaseSensitiveCompare> hashes_;
	};

}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const blobserver::Blob& b);
#endif

#endif /* __BLOBSERVER_BLOB_H__ */
