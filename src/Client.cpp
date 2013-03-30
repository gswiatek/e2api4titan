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

#include "Client.h"
#include "mongoose.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace gs::e2;

bool Client::get(const string& host, int port, const string& uri, string& data) {
	// TODO: log
	//cout << "get: " << host << ":" << port  << uri << endl;

	static const size_t bufSize = 4096;

	ostringstream os;

	os << "GET " << uri << " HTTP/1.1\r\n";
	os << "Host: " << host << "\r\n";
	os << "Connection: close\r\n";
	os << "User-Agent: GsE2TitanBridge/1.0\r\n";
	os << "\r\n";


	char buf[bufSize];
	struct mg_connection *conn = mg_download(host.c_str(), port, false, buf, bufSize, os.str().c_str());

	if (conn) {
		data.clear();
		data.reserve(bufSize * 2);

		int count = mg_read(conn, buf, bufSize);

		while (count > 0) {
			data.append(buf, count);

			count = mg_read(conn, buf, bufSize);
		}

		mg_close_connection(conn);

		return true;
	} else {
		// TODO: log
		//cout << "connection failed" << endl;
		//cout << buf << endl;

		return false;
	}
	
}
