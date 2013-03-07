
#include "Sync.hpp"

#include <sstream>
#include <string>

#include <curl/curl.h>

#include "json_spirit_reader.h"
#include "json_spirit_writer.h"
#include "json_spirit_utils.h"

#include "logger.hpp"

namespace blobserver {

	SyncEnumeration::SyncEnumeration() : set_last_(false) { }

	boost::optional<std::string> SyncEnumeration::last() {
		if (set_last_) {
			return boost::optional<std::string>(last_);
		}
		return boost::optional<std::string>();
	}

	void SyncEnumeration::last(std::string last) {
		last_ = last;
		set_last_ = true;
	}

	std::vector<std::string> SyncEnumeration::blob_refs() {
		return blob_refs_;
	}

	void SyncEnumeration::blob_refs(std::vector<std::string> blob_refs) {
		blob_refs_ = blob_refs;
	}

	void SyncEnumeration::add_blob_ref(std::string blob_ref) {
		blob_refs_.push_back(blob_ref);
	}

	Sync::Sync(int wait, std::vector<std::string> hosts, BlobIndex *bi) : wait_(wait), hosts_(hosts), bi_(bi), tick_(0), stopRequested_(false), thread_(boost::bind(&Sync::run, this)) {
		LOG_INFO("creating sync" << std::endl);
	}

	Sync::~Sync() {
		LOG_INFO("destroying sync" << std::endl);
	}

	void Sync::stop() {
		stopRequested_ = true;
		thread_.join();
		curl_global_cleanup();
	}

	void Sync::run() {
		curl_global_init(CURL_GLOBAL_ALL);

		LOG_INFO("sync running, wait=" << wait_ << std::endl);
		while (!stopRequested_) {
			if (++tick_ >= wait_) {
				LOG_INFO("tick" << std::endl);

				for (std::string &host : hosts_) {
					sync_host(host);
				}
				tick_ = 0;
			}

			sleep(1);
		}
	}

	void Sync::sync_host(std::string host) {
		bool get_more = true;
		std::string last = "";
		while (get_more) {
			std::string url = build_enumeration_url(host, last);
			LOG_INFO("getting url " << url << std::endl);
			boost::optional<std::string> content = fetch_blob_refs(url);
			if (content) {
				boost::optional<SyncEnumeration> sync_enumeration = parse_sync_enumeration(*content);
				if (sync_enumeration) {
					boost::optional<std::string> current_last = (*sync_enumeration).last();
					if (current_last) {
						last = *current_last;
					} else {
						get_more = false;
					}
				} else {
					get_more = false;
				}
			} else {
				get_more = false;
			}
		}
	}
 
	boost::optional<std::string> Sync::fetch_blob_refs(std::string url) {
		std::string buffer;
		char errorBuffer[CURL_ERROR_SIZE];

		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "blobserver/1.0.0");
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

		CURLcode result = curl_easy_perform(curl);

		curl_easy_cleanup(curl);

		if (result == CURLE_OK) {
			LOG_INFO("got back " << std::endl << buffer << std::endl);
			return boost::optional<std::string>(buffer);
		}
		LOG_ERROR("Curl result was not CURL_OK: " << result << " - " << errorBuffer << std::endl);
		return boost::optional<std::string>();
	}

	boost::optional<SyncEnumeration> Sync::parse_sync_enumeration(std::string content) {
		json_spirit::Value value;
		bool result = json_spirit::read(content, value);
		if (result == false) {
			return boost::optional<SyncEnumeration>();
		}
		if (value.type() == json_spirit::Value_type::obj_type) {
			const json_spirit::Object& obj = value.get_obj();

			SyncEnumeration sync_enumeration;
			for (json_spirit::Object::const_iterator i = obj.begin(); i != obj.end(); ++i) {
				const std::string& name = i->name_;
				const json_spirit::Value& value = i->value_;

				if (name == "continueAfter") {
					sync_enumeration.last(value.get_str());
				}

				if (name == "blobs") {
					const json_spirit::Array& blobs_array = value.get_array();
					for (unsigned int j = 0; j < blobs_array.size(); ++j) {
						const json_spirit::Object& blob_obj = blobs_array[j].get_obj();
						for (json_spirit::Object::const_iterator blobs_obj_iter = blob_obj.begin(); blobs_obj_iter != blob_obj.end(); ++blobs_obj_iter) {
							const std::string& name = blobs_obj_iter->name_;
							const json_spirit::Value& value = blobs_obj_iter->value_;
							if (name == "blobRef") {
								sync_enumeration.add_blob_ref(value.get_str());
							}
						}
					}
				}

			}
			return boost::optional<SyncEnumeration>(sync_enumeration);
		}

		return boost::optional<SyncEnumeration>();
	}

	std::string Sync::build_enumeration_url(std::string host, std::string last) {
		std::stringstream ss;
		ss << host << "/enumerate-blobs";
		if (last.length() > 0) {
			ss << "?after=" << last;
		}
		return ss.str();
	}

	int writer(char *data, size_t size, size_t nmemb, std::string *buffer) {
		int result = 0;
		if (buffer != NULL) {
			buffer->append(data, size * nmemb);
			result = size * nmemb;
		}
		return result;
	}

}
