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

#include "FileHelper.h"
#include "LineHandler.h"
#include "Log.h"
#include <iostream>
#include <fstream>

using namespace gs::e2;
using namespace std;

void FileHelper::readConfigFile(const string& fileName, LineHandler& handler) {
	Log* log = Log::getLogger();

	log->log(Log::DEBUG, "ADP", "read: " + fileName);
	ifstream is(fileName.c_str());

	if (is) {
		handleStream(is, handler);
	} else {
		log->log(Log::ERROR, "ADP", "open failed for: " + fileName);
	}

	is.close();
}

void FileHelper::handleStream(istream& is, LineHandler& handler) {
	static const int bufSize = 4096;

	char buf[bufSize];

	int count = readLine(is, buf, bufSize);

	vector<char*> line;
	int pos;
	char* data;

	while (count != -1) {
		line.clear();
		pos = 0;
		data = buf;

		while (pos < count - 1) {
			if (buf[pos] == '#') {
				buf[pos] = 0;
				line.push_back(data);
				data = buf + pos + 1;
			}

			++pos;
		}

		line.push_back(data);
		handler.handleLine(line);

		count = readLine(is, buf, bufSize);
	}

	handler.finished();
}
