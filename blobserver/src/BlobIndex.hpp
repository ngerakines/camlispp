#ifndef __BLOBSERVER_BLOB_INDEX_H__
#define __BLOBSERVER_BLOB_INDEX_H__

#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/thread/mutex.hpp>

#include "logger.hpp"
#include "Blob.hpp"
#include "Config.hpp"

namespace blobserver {

	class BlobIndex {
		public:
			BlobIndex(Config *config);

			~BlobIndex();

			void addBlob(std::vector<char> bytes);

			bool empty() {
				boost::mutex::scoped_lock lock(mutex_);
				return blobs_.empty();
			}

			int size() {
				boost::mutex::scoped_lock lock(mutex_);
				return (int) blobs_.size();
			}

			Blob* get(std::string hash);

		private:
			Config *config_;
			std::vector<Blob *> blobs_;
			mutable boost::mutex mutex_;
	};

}

#endif /* __BLOBSERVER_BLOB_INDEX_H__ */
