#ifndef __SYNC_HPP__
#define __SYNC_HPP__

#include <string>
#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/optional.hpp>

#include <curl/curl.h>

#include "BlobIndex.hpp"

namespace blobserver {

	class SyncEnumeration {
		public:
			SyncEnumeration();

			boost::optional<std::string> last();
			void last(std::string last);

			std::vector<std::string> blob_refs();
			void blob_refs(std::vector<std::string> blob_refs);
			void add_blob_ref(std::string blob_ref);
		private:
			bool set_last_;
			std::string last_;
			std::vector<std::string> blob_refs_;
	};

	class Sync {
		public:
			Sync(Config *ci, BlobIndex *bi);
			~Sync();

			void stop();

		private:
			Config *ci_;
			BlobIndex *bi_;

			int tick_;
			volatile bool stopRequested_;
			boost::thread thread_;
			boost::mutex mutex_;

			void run();
			void sync_host(SyncConfig sync_config);
			boost::optional<SyncEnumeration> parse_sync_enumeration(std::string content);
			boost::optional<std::string> fetch_url(CURL *curl, std::string url);
			void fetch_blob(CURL *curl, std::string host, std::string blob_ref);
			std::string build_enumeration_url(std::string host, std::string last);
			std::string build_blob_url(std::string host, std::string blob_ref);
	};

	int writer(char *data, size_t size, size_t nmemb, std::string *buffer);
}

#endif
