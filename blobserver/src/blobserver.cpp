/*
** Copyright (c) 2013 Nick Gerakines
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/

#include <signal.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/archive/tmpdir.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include "server.hpp"
#include "Config.hpp"
#include "BlobIndex.hpp"

using namespace std;
namespace po = boost::program_options;

using namespace blobserver;

volatile bool running;

void sighandler(int signum) {
	std::cout << "Received signal " << signum << std::endl;

	if (signum == SIGINT) {
		running = false;
	}
}

void save_blob_index(const BlobIndex &bi, const char * filename) {
	// make an archive
	std::ofstream ofs(filename);
	boost::archive::text_oarchive oa(ofs);
	oa << bi;
}

int main(int argc, char **argv, char ** /* **ppenv */) {
	po::options_description desc("Allowed options");
	desc.add_options()
	("help", "produce help message")
	("directory", po::value<string>(), "The directory to save blobs in and serve them from")
	("ip", po::value<string>(), "The ip address to bind to")
	("port", po::value<string>(), "The port to serve requests on")
	("threads", po::value<int>(), "The number of threads to use")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);
	Config config;

	if (vm.count("directory")) {
		config.directory(vm["directory"].as<string>());
	}

	if (vm.count("ip")) {
		config.ip(vm["ip"].as<string>());
	}

	if (vm.count("port")) {
		config.port(vm["port"].as<string>());
	}

	if (vm.count("threads")) {
		config.threads(vm["threads"].as<int>());
	}

	if (vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	running = true;
	signal(SIGINT, sighandler);
	// create some fake bytes
	char buf[] = {0, 1, 2, 3, 4, 5};
	char buf2[] = {5, 4, 3, 2, 1, 6, 7, 8, 9};
	std::vector<char> vec(buf, buf + sizeof(buf) / sizeof(buf[0]));
	std::vector<char> vec2(buf2, buf2 + sizeof(buf2) / sizeof(buf2[0]));
	BlobIndex bi(&config);
	bi.add_blob(&vec);
	bi.add_blob(&vec2);

	std::string filename(boost::archive::tmpdir());
	filename += "/blobindex.txt";
	save_blob_index(bi, filename.c_str());

	try {
		// Initialise the server.
		http::server3::server s(&bi, config.ip().c_str(), config.port(), config.directory().c_str(), config.threads());
		// Run the server until stopped.
		s.run();

	} catch (std::exception& e) {
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}
