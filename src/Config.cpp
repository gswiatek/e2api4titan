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
#include "Properties.h"

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
char Config::fileSeparator('\\');
string Config::m_titanDir("c:\\users\\swiatek\\documents\\SatReceiver\\development\\titan\\");
string Config::m_etcDir("c:\\users\\swiatek\\documents\\SatReceiver\\development\\etc\\");
#else
char Config::fileSeparator('/');
string Config::m_titanDir("/mnt/settings/");
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
		} else if (strcmp(argv[pos], "-c") == 0) {
			if (pos + 1 == argc) {
				usage(argv[0]);
				return false;
			}

			string fileName = argv[++pos];

			Properties props;

			if (props.read(fileName)) {
				const string& port = props.get("port");

				if (!port.empty()) {
					m_port = Util::getInt(port);
				}

				const string& dataPort = props.get("dataPort");

				if (!dataPort.empty()) {
					m_dataPort = Util::getInt(dataPort);
				}

				const string& titanPort = props.get("titanPort");

				if (!titanPort.empty()) {
					m_titanPort = Util::getInt(titanPort);
				}

				const string& titanDataPort = props.get("titanDataPort");

				if (!titanDataPort.empty()) {
					m_titanDataPort = Util::getInt(titanDataPort);
				}
				
				const string& titanHost = props.get("titanHost");

				if (!titanHost.empty()) {
					m_titanHost = titanHost;
				}

				const string& titanDir = props.get("titanDir");

				if (!titanDir.empty()) {
					m_titanDir = titanDir;

					if (titanDir.at(titanDir.length() -1) != fileSeparator) {
						m_titanDir.append(1, fileSeparator);
					}
				}

				const string& autoZap = props.get("autoZap");

				if (!autoZap.empty()) {
					m_autoZap = ("true" == autoZap) || ("1" == autoZap);
				}

				const string& logFile = props.get("logFile");

				if (!logFile.empty()) {
					m_logFile = logFile;
				}

				const string& maxLogSize = props.get("maxLogSize");

				if (!maxLogSize.empty()) {
					m_maxLogFileSize = Util::getInt(maxLogSize);
				}

				const string& threadPool = props.get("threadPool");

				if (!threadPool.empty()) {
					int threads = Util::getInt(threadPool);

					if (threads > 0 && threads <= 8) {
						m_threads = threads;
					}

				}
			}

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
	cerr << "\t-c <file>    Configuration file" << endl;
}
