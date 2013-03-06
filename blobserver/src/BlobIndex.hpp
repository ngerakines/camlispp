#ifndef __BLOBSERVER_BLOB_INDEX_H__
#define __BLOBSERVER_BLOB_INDEX_H__

#include <string>
#include <map>
#include <vector>

#include <boost/thread/mutex.hpp>

#include "Config.hpp"
#include "Blob.hpp"
#include "BlobKey.hpp"
#include "Hash.hpp"

namespace blobserver {

	class BlobIndex {
		public:
			explicit BlobIndex(Config *config);

			~BlobIndex();

			Blob* add_blob(std::vector<char> *bytes);
			Blob* add_blob(std::vector<char> *bytes, std::vector<HashType> hash_types);

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
	};

	std::string blob_filename(std::string hash);

}

#endif /* __BLOBSERVER_BLOB_INDEX_H__ */
