
#include "Sync.hpp"

#include <sstream>
#include <string>

#include <boost/algorithm/string/predicate.hpp>

#include "json_spirit_reader.h"
#include "json_spirit_writer.h"
#include "json_spirit_utils.h"

#include "config.h"
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

	Sync::Sync(Config *ci, BlobIndex *bi) : ci_(ci), bi_(bi), tick_(0), stopRequested_(false), thread_(boost::bind(&Sync::run, this)) {
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

		while (!stopRequested_) {
			if (++tick_ >= ci_->sync_delay()) {
				for (SyncConfig &sync_config : ci_->sync_servers()) {
					sync_host(sync_config);
				}
				tick_ = 0;
			}

			sleep(1);
		}
	}

	void Sync::sync_host(SyncConfig sync_config) {
		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "blobserver/1.0.0");
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 0);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		if (sync_config.get()) {
			bool get_more = true;
			std::string last = "";
			while (get_more) {
				std::string url = build_enumeration_url(sync_config.host(), last);
				boost::optional<std::string> content = fetch_url(curl, url);
				if (content) {
					boost::optional<SyncEnumeration> sync_enumeration = parse_sync_enumeration(*content);
					if (sync_enumeration) {
						for (std::string &blob_ref : (*sync_enumeration).blob_refs()) {
							Blob *blob = bi_->get(blob_ref);
							if (blob == NULL) {
								fetch_blob(curl, sync_config.host(), blob_ref);
							}
						}

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
		if (sync_config.send()) {
			bool check_more = true;
			boost::optional<std::string> after = boost::optional<std::string>();
			while (check_more) {
				std::vector<std::pair<BlobKey, Blob*>> blobs;
				bi_->paginate(&blobs, after, 501);
				if (blobs.size() == 501) {
					auto last = blobs.back();
					after.reset(last.first.blobref());
				} else {
					check_more = false;
				}

				if (blobs.size() > 0) {
					std::set<std::string> check_blob_refs;
					for (auto &pair : blobs) {
						check_blob_refs.insert(pair.first.blobref());
					}
					std::string stat_request_body = stat_request(blobs);
					std::string url = build_stat_url(sync_config.host());
					boost::optional<std::string> content = fetch_url(curl, url, stat_request_body);
					if (content) {
						boost::optional<SyncEnumeration> sync_enumeration = parse_sync_enumeration(*content);
						if (sync_enumeration) {
							for (std::string &blob_ref : (*sync_enumeration).blob_refs()) {
								check_blob_refs.erase(blob_ref);
							}
							LOG_INFO("The following blobs should be sent:" << std::endl);
							for (auto &blob_ref : check_blob_refs) {
								LOG_INFO(" - " << blob_ref << std::endl);
								send_blob(curl, sync_config.host(), blob_ref);
							}
						} else {
							check_more = false;
						}
					} else {
						check_more = false;
					}
				}
			}
		}
		curl_easy_cleanup(curl);
		// delete curl;
	}

	boost::optional<std::string> Sync::send_blob(CURL *curl, std::string host, std::string blob_ref) {
		Blob *blob = bi_->get(blob_ref);
		if (blob == NULL) {
			return boost::optional<std::string>();
		}

		std::string url = host + "/upload";
		LOG_INFO("fetching url " << url << std::endl);

		curl_httppost* post = build_upload(blob_ref, blob);

		std::string buffer;
		char errorBuffer[CURL_ERROR_SIZE];

		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

		CURLcode result = curl_easy_perform(curl);

		curl_formfree(post);
		curl_easy_reset(curl);

		if (result == CURLE_OK) {
			return boost::optional<std::string>(buffer);
		}
		LOG_ERROR("Curl result was not CURL_OK: " << result << " - " << errorBuffer << std::endl);
		return boost::optional<std::string>();
	}

	curl_httppost* Sync::build_upload(std::string blob_ref, Blob* blob) {
		struct curl_httppost* post = NULL;
		struct curl_httppost* last = NULL;

		/* Add name/ptrcontent/contenttype section */
		curl_formadd(&post, &last,
			// Content-Disposition: form-data; name="sha1-9b03f7aca1ac60d40b5e570c34f79a3e07c918e8"; filename="blob1"
			CURLFORM_COPYNAME, blob_ref.c_str(),
			CURLFORM_FILE, blob->filePath().c_str(),
			CURLFORM_FILENAME, "blob1",
			CURLFORM_CONTENTTYPE, "application/octet-stream", CURLFORM_END);

		LOG_INFO("creating form post of " << blob_ref << " with " << blob->filePath() << " as 'blob1' and 'application/octet-stream'" << std::endl);
		return post;
	}

	std::string Sync::stat_request(std::vector<std::pair<BlobKey, Blob*>> blobs) {
		std::stringstream ss;
		ss << "camliversion=" << CAMLI_VERSION << "&";
		int n = 1;
		for (auto &pair : blobs) {
			ss << "&blob" << n++ << "=" << pair.first.blobref();
		}
		return ss.str();
	}
 
	boost::optional<std::string> Sync::fetch_url(CURL *curl, std::string url) {
		LOG_INFO("fetching url " << url << std::endl);

		std::string buffer;
		char errorBuffer[CURL_ERROR_SIZE];

		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

		CURLcode result = curl_easy_perform(curl);

		curl_easy_reset(curl);

		if (result == CURLE_OK) {
			return boost::optional<std::string>(buffer);
		}
		LOG_ERROR("Curl result was not CURL_OK: " << result << " - " << errorBuffer << std::endl);
		return boost::optional<std::string>();
	}

	boost::optional<std::string> Sync::fetch_url(CURL *curl, std::string url, std::string postbody) {
		LOG_INFO("fetching url " << url << std::endl);

		std::string buffer;
		char errorBuffer[CURL_ERROR_SIZE];

		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postbody.c_str());

		CURLcode result = curl_easy_perform(curl);

		curl_easy_reset(curl);

		if (result == CURLE_OK) {
			return boost::optional<std::string>(buffer);
		}
		LOG_ERROR("Curl result was not CURL_OK: " << result << " - " << errorBuffer << std::endl);
		return boost::optional<std::string>();
	}

	void Sync::fetch_blob(CURL *curl, std::string host, std::string blob_ref) {
		std::string url = build_blob_url(host, blob_ref);
		LOG_INFO("fetching url " << url << std::endl);

		boost::optional<std::string> blob_ref_content = fetch_url(curl, url);
		if (blob_ref_content) {
			std::vector<char> charvect((*blob_ref_content).begin(), (*blob_ref_content).end());
			bi_->add_blob(boost::optional<std::string>(blob_ref), &charvect);
		}
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

				if (name == "blobs" || name == "stat") {
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

	std::string Sync::build_stat_url(std::string host) {
		std::stringstream ss;
		ss << host << "/stat";
		return ss.str();
	}

	std::string Sync::build_enumeration_url(std::string host, std::string last) {
		std::stringstream ss;
		ss << host << "/enumerate-blobs";
		if (last.length() > 0) {
			ss << "?after=" << last;
		}
		return ss.str();
	}

	std::string Sync::build_blob_url(std::string host, std::string blob_ref) {
		std::stringstream ss;
		ss << host;
		if (!boost::ends_with(host, "/")) {
			ss << "/";
		}
		ss << blob_ref;
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
