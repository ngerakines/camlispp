#ifndef __BLOBSERVER_CONFIG_H__
#define __BLOBSERVER_CONFIG_H__

#include <string>
#include <vector>
#include "config.h"

namespace blobserver {

	enum SyncMode : int {
	    get = 1,
	    send = 2,
	    get_and_send = 4
	};

	class SyncConfig {
		public:
			explicit SyncConfig(std::string host);
			explicit SyncConfig(std::string host, SyncMode mode);

			std::string host();
			SyncMode mode();

			bool get();
			bool send();

		private:
			std::string host_;
			SyncMode mode_;
	};

	class Config {

		public:
			explicit Config();
			~Config();

			void directory(std::string directory);
			std::string directory();

			void ip(std::string ip);
			std::string ip();

			std::string port();
			void port(std::string port);

			int threads();
			void threads(int threads);

			int sync_delay();
			void sync_delay(int sync_delay);

			bool validate();
			void validate(bool validate);

			std::vector<SyncConfig> sync_servers();
			void sync_servers(std::vector<std::string> sync_servers);

#if defined ENABLE_STATIC
			std::string static_directory();
			void static_directory(std::string static_directory);
#endif

		private:
			std::string directory_;
			std::string ip_;
			std::string port_;
			int threads_;
			bool validate_;
			int sync_delay_;

			std::vector<SyncConfig> sync_servers_;

#if defined ENABLE_STATIC
			std::string static_directory_;
#endif

	};

}

#endif
