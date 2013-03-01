#ifndef __BLOBSERVER_CONFIG_H__
#define __BLOBSERVER_CONFIG_H__

#include <string>

namespace blobserver {

	class Config {

		public:
			explicit Config() : directory_("/tmp/"), ip_("0.0.0.0"), port_("8080"), threads_(2) { }
			~Config() { }

			void directory(std::string directory) {
				directory_ = directory;
			}

			std::string directory() {
				return directory_;
			}

			void ip(std::string ip) {
				ip_ = ip;
			}

			std::string ip() {
				return ip_;
			}

			std::string port() {
				return port_;
			}

			void port(std::string port) {
				port_ = port;
			}

			int threads() {
				return threads_;
			}

			void threads(int threads) {
				threads_ = threads;
			}

		private:
			std::string directory_;
			std::string ip_;
			std::string port_;
			int threads_;
	};

}

#endif
