#ifndef __BLOBSERVER_BLOB_INDEX_H__
#define __BLOBSERVER_BLOB_INDEX_H__

#include <string>
#include <map>
#include <vector>

#include <boost/optional.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

#include "Config.hpp"
#include "Blob.hpp"
#include "BlobKey.hpp"
#include "Hash.hpp"

namespace blobserver {

	class BlobIndex {
		public:
			explicit BlobIndex(Config *config);

			~BlobIndex();

			boost::optional<Blob*> add_blob(boost::optional<std::string> provided_hash, std::vector<char> *bytes);
			boost::optional<Blob*> add_blob(boost::optional<std::string> provided_hash, std::vector<char> *bytes, std::vector<HashType> hash_types);

			bool empty();

			int size();

			Blob* get(std::string hash);

			void paginate(std::vector<std::pair<BlobKey, Blob*>> *blobs);
			void paginate(std::vector<std::pair<BlobKey, Blob*>> *blobs, boost::optional<std::string> last, int count);

			std::string create_path(std::string hash);

		private:
			Config *config_;
			std::map<BlobKey, Blob*> blobs_;
			mutable boost::mutex mutex_;

			friend class boost::serialization::access;

			// When the class Archive corresponds to an output archive, the
			// & operator is defined similar to <<.  Likewise, when the class Archive
			// is a type of input archive the & operator is defined similar to >>.
			template<class Archive>
			void serialize(Archive & ar, const unsigned int version) {
				ar & blobs_;
			}
	};

	std::string blob_filename(std::string hash);

}

#endif /* __BLOBSERVER_BLOB_INDEX_H__ */
