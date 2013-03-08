
#include "Config.hpp"

namespace blobserver {

	SyncConfig::SyncConfig(std::string host) : SyncConfig(host, SyncMode::get_and_send) {

	}

	SyncConfig::SyncConfig(std::string host, SyncMode mode) : host_(host), mode_(mode) {

	}

	std::string SyncConfig::host() {
		return host_;
	}

	SyncMode SyncConfig::mode() {
		return mode_;
	}

	bool SyncConfig::get() {
		if (mode_ == SyncMode::get || mode_ == SyncMode::get_and_send) {
			return true;
		}
		return false;
	}

	bool SyncConfig::send() {
		if (mode_ == SyncMode::send || mode_ == SyncMode::get_and_send) {
			return true;
		}
		return false;
	}

	Config::Config() : directory_("/tmp/"), ip_("0.0.0.0"), port_("8080"), threads_(2), validate_(true), sync_delay_(60) { }

	Config::~Config() { }

	void Config::directory(std::string directory) {
		directory_ = directory;
	}

	std::string Config::directory() {
		return directory_;
	}

	void Config::ip(std::string ip) {
		ip_ = ip;
	}

	std::string Config::ip() {
		return ip_;
	}

	std::string Config::port() {
		return port_;
	}

	void Config::port(std::string port) {
		port_ = port;
	}

	int Config::threads() {
		return threads_;
	}

	void Config::threads(int threads) {
		threads_ = threads;
	}

	int Config::sync_delay() {
		return sync_delay_;
	}

	void Config::sync_delay(int sync_delay) {
		sync_delay_ = sync_delay;
	}

	bool Config::validate() {
		return validate_;
	}

	void Config::validate(bool validate) {
		validate_ = validate;
	}

	std::vector<SyncConfig> Config::sync_servers() {
		return sync_servers_;
	}

	void Config::sync_servers(std::vector<std::string> sync_servers) {
		for (std::string &sync_server : sync_servers) {
			sync_servers_.push_back(SyncConfig(sync_server));
		}
	}

#if defined ENABLE_STATIC
	void Config::static_directory(std::string static_directory) {
		static_directory_ = static_directory;
	}

	std::string Config::static_directory() {
		return static_directory_;
	}
#endif

}

