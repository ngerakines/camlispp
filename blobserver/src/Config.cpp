
#include "Config.hpp"

namespace blobserver {

	Config::Config() : directory_("/tmp/"), ip_("0.0.0.0"), port_("8080"), threads_(2), validate_(true) { }
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

	bool Config::validate() {
		return validate_;
	}

	void Config::validate(bool validate) {
		validate_ = validate;
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

