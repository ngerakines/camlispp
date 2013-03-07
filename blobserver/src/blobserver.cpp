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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "config.h"
#include "server.hpp"
#include "Config.hpp"
#include "BlobIndex.hpp"
#include "Sync.hpp"

void save_blob_index(const blobserver::BlobIndex &bi, const char * filename) {
	// make an archive
	std::ofstream ofs(filename);
	boost::archive::text_oarchive oa(ofs);
	oa << bi;
}

int main(int argc, char **argv, char **) {
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
	("help", "produce help message")
	("directory", boost::program_options::value<std::string>(), "The directory to save blobs in and serve them from")
	("ip", boost::program_options::value<std::string>(), "The ip address to bind to")
	("port", boost::program_options::value<std::string>(), "The port to serve requests on")
	("threads", boost::program_options::value<int>(), "The number of threads to use")
	("sync", boost::program_options::value<std::vector<std::string>>(), "One or more servers to sync with")
	("sync-delay", boost::program_options::value<int>(), "The minimum amount of time in seconds between syncs")
#if defined ENABLE_STATIC
	("static_directory", boost::program_options::value<std::string>(), "The directory to serve static files from")
#endif
#if defined ENABLE_DEBUG_LOAD
	("load_directory", boost::program_options::value<std::string>(), "The directory to load data from")
#endif
	;
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);
	blobserver::Config config;

	if (vm.count("directory")) {
		config.directory(vm["directory"].as<std::string>());
	}

#if defined ENABLE_STATIC

	if (vm.count("static_directory")) {
		config.static_directory(vm["static_directory"].as<std::string>());

	} else {
		config.static_directory("./static/");
	}

#endif
#if defined ENABLE_DEBUG_LOAD
	std::string load_directory = "";

	if (vm.count("load_directory")) {
		load_directory = vm["load_directory"].as<std::string>();
	}

#endif

	if (vm.count("ip")) {
		config.ip(vm["ip"].as<std::string>());
	}

	if (vm.count("port")) {
		config.port(vm["port"].as<std::string>());
	}

	if (vm.count("sync")) {
		config.sync_servers(vm["sync"].as<std::vector<std::string>>());
	}

	if (vm.count("threads")) {
		config.threads(vm["threads"].as<int>());
	}

	if (vm.count("sync-delay")) {
		config.sync_delay(vm["sync-delay"].as<int>());
	}

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	blobserver::BlobIndex bi(&config);
#if defined ENABLE_DEBUG_LOAD
	boost::filesystem::path path(load_directory);

	if (boost::filesystem::exists(path) && boost::filesystem::is_directory(path)) {
		LOG_INFO("Loading files from " << path << std::endl);
		typedef std::vector<boost::filesystem::path> vec;
		vec v;
		copy(boost::filesystem::directory_iterator(path), boost::filesystem::directory_iterator(), back_inserter(v));

		for (auto & file: v) {
			if (boost::filesystem::is_regular_file(file)) {
				LOG_INFO("path=" << file << "; filename=" << file.filename() << std::endl);
				std::ifstream is(file.c_str(), std::ios::in | std::ios::binary);

				if (is) {
					char buf[512];
					std::string content;

					while (is.read(buf, sizeof(buf)).gcount() > 0) {
						content.append(buf, is.gcount());
					}

					LOG_INFO("content was read to be " << content.length() << std::endl);
					std::vector<char> charvect(content.begin(), content.end());
					bi.add_blob(boost::optional<std::string>(), &charvect);
				}
			}
		}
	}

#endif
	std::string filename(boost::archive::tmpdir());
	filename += "/blobindex.txt";
	save_blob_index(bi, filename.c_str());

	blobserver::Sync sync(config.sync_delay(), config.sync_servers(), &bi);

	try {
		// Initialise the server.
		http::server3::server s(&config, &bi, config.ip().c_str(), config.port(), config.threads());
		// Run the server until stopped.
		s.run();

	} catch (std::exception& e) {
		std::cerr << "exception: " << e.what() << std::endl;
	}

	sync.stop();

	return 0;
}
