// Copyright (c) 2013, Grzegorz Swiatek. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Grzegorz Swiatek nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Config.h"
#include "Util.h"

#include <cstring>
#include <iostream>

using namespace std;
using namespace gs::e2;

bool Config::m_daemon = false;
int Config::m_port = 8080;
int Config::m_dataPort = 8001;
int Config::m_threads = 2;

int Config::m_titanPort = 80;
int Config::m_titanDataPort = 22222;

string Config::m_titanHost = "127.0.0.1";

#ifdef _WIN32
string Config::m_titanDir("c:\\users\\swiatek\\documents\\SatReceiver\\development\\titan\\");
string Config::m_etcDir("c:\\users\\swiatek\\documents\\SatReceiver\\development\\etc\\");
#else
string Config::m_titanDir("/var/etc/titan/");
string Config::m_etcDir("/etc/");
#endif

string Config::m_logFile("/tmp/e2webserv.log");
size_t Config::m_maxLogFileSize(10);

bool Config::m_autoZap = false;

bool Config::m_defaultRecFileNameFormat = true;

const bool& Config::isDaemon() {
	return m_daemon;
}

const int& Config::getPort() {
	return m_port;
}

const int& Config::getDataPort() {
	return m_dataPort;
}

const int& Config::getThreads() {
	return m_threads;
}

const string& Config::getTitanHost() {
	return m_titanHost;
}

const int& Config::getTitanPort() {
	return m_titanPort;
}

const int& Config::getTitanDataPort() {
	return m_titanDataPort;
}

const string& Config::getTitanDir() {
	return m_titanDir;
}


const string& Config::getEtcDir() {
	return m_etcDir;
}

const string& Config::getLogFile() {
	return m_logFile;
}

size_t Config::getMaxLogFileSize() {
	return m_maxLogFileSize;
}

const bool& Config::isAutoZap() {
	return m_autoZap;
}

const bool& Config::isDefaultRecFileNameFormat() {
	return m_defaultRecFileNameFormat;
}

void Config::setDefaultRecFileNameFormat(bool val) {
	m_defaultRecFileNameFormat = val;
}

bool Config::parse(int argc, const char** argv) {

	int pos = 1;

	while (pos < argc) {
		if (strcmp(argv[pos], "-b") == 0) {
			m_daemon = true;
		} else if (strcmp(argv[pos], "-a") == 0) {
			m_autoZap = true;
		} else if (strcmp(argv[pos], "-p") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}

			m_port = Util::getInt(argv[++pos]);
		} else if (strcmp(argv[pos], "-h") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}
			m_titanHost = argv[++pos];
		} else if (strcmp(argv[pos], "-d") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}
			m_titanDir = argv[++pos];
#ifndef _WIN32
			if (m_titanDir[m_titanDir.size() - 1] != '/') {
				m_titanDir.append("/");
			}
#else
			if (m_titanDir[m_titanDir.size() - 1] != '\\') {
				m_titanDir.append("\\");
			}
#endif
		} else if (strcmp(argv[pos], "-dp") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}
			m_dataPort = Util::getInt(argv[++pos]);
		} else if (strcmp(argv[pos], "-tp") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}
			m_titanPort = Util::getInt(argv[++pos]);
		} else if (strcmp(argv[pos], "-tdp") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}
			m_titanDataPort = Util::getInt(argv[++pos]);
		} else if (strcmp(argv[pos], "-n") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}
			m_threads = Util::getInt(argv[++pos]);
		} else {
			usage(argv[0]);
			return false;
		}

		++pos;
	}

	return true;
}

void Config::usage(const char* progName) {
	cerr << progName << "[<option>*]" << endl;
	cerr << "Options: " << endl;
	cerr << "\t-b           Start the program in background" << endl;
	cerr << "\t-a           Enable auto zap" << endl;
	cerr << "\t-p <num>     Web Server port (default: 8080)" << endl;
	cerr << "\t-dp <num>    E2 specific data port (default: 8001)" << endl;
	cerr << "\t-h <ip>      IP of Titan Receiver (default: 127.0.0.1)" << endl;
	cerr << "\t-tp <num>    Titan Web Server port (default: 80)" << endl;
	cerr << "\t-tdp <num>   Titan data port (default: 22222)" << endl;
	cerr << "\t-d <dir>     Titan settings directory (default: /var/etc/titan/)" << endl;
	cerr << "\t-n <num>     Number of threads in pool (default: 2)" << endl;
}
