#ifndef __BLOBSERVER_BLOB_H__
#define __BLOBSERVER_BLOB_H__

#include <cstring>
#include <string>
#include <vector>
#include <set>
#if defined ENABLE_DUMP
#include <iostream>
#endif

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>

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

	typedef std::set<std::string, CaseSensitiveCompare> sset;

	class Blob final {
		public:
			explicit Blob(std::string filePath);
			~Blob();

			std::string filePath();

			int size();
			void size(int size);

			void add_hash(std::string hash);
			sset hashes();

			bool is_match(std::string hash);

#if defined ENABLE_DUMP
			std::ostream& dump(std::ostream& o) const;
#endif
		private:
			std::string filePath_;
			int size_;
			sset hashes_;

			friend class boost::serialization::access;

			// When the class Archive corresponds to an output archive, the
			// & operator is defined similar to <<.  Likewise, when the class Archive
			// is a type of input archive the & operator is defined similar to >>.
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version) {
				ar & filePath_;
				ar & size_;
				ar & hashes_;
			}
	};

}

#if defined ENABLE_DUMP
std::ostream& operator<<(std::ostream& o, const blobserver::Blob& b);
#endif

#endif /* __BLOBSERVER_BLOB_H__ */
