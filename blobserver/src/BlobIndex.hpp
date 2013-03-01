#ifndef __BLOBSERVER_BLOB_INDEX_H__
#define __BLOBSERVER_BLOB_INDEX_H__

#include <string>
#include <vector>

#include <boost/thread/mutex.hpp>

#include "Blob.hpp"
#include "Config.hpp"

namespace blobserver {

	class BlobIndex {
		public:
			BlobIndex(Config *config);

			~BlobIndex();

			void addBlob(std::vector<char> bytes);

			bool empty();

			int size();

			Blob* get(std::string hash);

		private:
			Config *config_;
			std::vector<Blob *> blobs_;
			mutable boost::mutex mutex_;
	};

}

#endif /* __BLOBSERVER_BLOB_INDEX_H__ */
