#ifndef __BLOBSERVER_CONFIG_H__
#define __BLOBSERVER_CONFIG_H__

#include <string>

namespace blobserver {

	class Config {

		public:
			explicit Config();
			~Config();

			void directory(std::string directory);
			std::string directory();

			void ip(std::string ip);
			std::string ip() ;

			std::string port();
			void port(std::string port);

			int threads();
			void threads(int threads);

			bool validate();
			void validate(bool validate);

		private:
			std::string directory_;
			std::string ip_;
			std::string port_;
			int threads_;
			bool validate_;
	};

}

#endif
