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

#include "mongoose.h"
#include "Config.h"
#include "TitanAdapter.h"
#include "Util.h"
#include "Version.h"

#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#ifndef _WIN32
#include <sys/stat.h>
#endif

#include <cerrno>
#include <cstring>


using namespace std;
using namespace gs::e2;


/*void writeChunk(struct mg_connection*conn, const string& data) {
	int len = data.length();
	const char* buf = data.c_str();

	int pos = 0;
	int chunkSize = 512;

	while (pos + chunkSize < len) {
		mg_write(conn, "ff\r\n", 4);
		mg_write(conn, buf + pos, chunkSize);
		mg_write(conn, "\r\n", 2);
		pos += chunkSize;
	}
	

	if (pos < len) {
		char tmp[20];

#ifdef _WIN32
		_snprintf_s(tmp, 20, "%x\r\n", (len - pos));
#else
		snprintf(tmp, 20, "%x\r\n", (len - pos));
#endif
		mg_write(conn, tmp, strlen(tmp));
		mg_write(conn, buf + pos, (len - pos));
		mg_write(conn, "\r\n", 2);
	}

	mg_write(conn, "0\r\n\r\n\r\n", 7);
}*/

string movedTemporarily(struct mg_connection* conn, ostringstream& response) {
	string server;

	const char* host = mg_get_header(conn, "Host");

	if (host) {
		response << "HTTP/1.1 302 Moved Temporarily\r\n";
	} else {
		response << "HTTP/1.1 400 Bad Request\r\n";
	}

	response << "Date: " << Util::getHttpDate() << "\r\n";

	if (host) {
		const string& titanHost = Config::getTitanHost();

		if (titanHost != "127.0.0.1") { // we are not local -> use configured titan host
			server = titanHost.c_str();
		} else {
			server = host;

			string::size_type pos = server.find(':');

			if (pos != string::npos) {
				server = server.substr(0, pos);
			}
		}
	}

	return server;
}

void handle(struct mg_connection* conn, std::string& uri, const string& query) {
	static const string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

	// TODO: log
	//cout << "handle: uri=" << uri << ", query=" << query << endl;

	bool notFound = false;

	ostringstream os;

	if (uri.find("/web/") == 0) {

		uri = uri.substr(5);

		if (uri == "zap" && query.find("sRef=") == 0) {
			string serviceRef = Util::getTitanRef(query.substr(5));
			TitanAdapter::getAdapter()->zap(serviceRef);

			os << xml << endl;
			os << "<rootElement></rootElement>" << endl;
		} else if (uri == "getcurrent") {
			CurrentService cs = TitanAdapter::getAdapter()->getCurrent();

			os << xml << endl;
			os << cs;
		} else if (uri == "serviceplayable") {
			string sref;
			string playing; // TODO:

			if (query.find("sRef=") == 0) {
				sref = query.substr(5);
			}

			string::size_type pos = sref.find("&sRefPlaying=");

			if (pos != string::npos) {
				playing = sref.substr(pos + 13);
				sref = sref.substr(0, pos);
			}

			bool res = (sref.find("1:0:1") == 0);
			os << xml << endl;
			os << "<e2serviceplayable>";
			os << "<e2servicereference>"  << sref << "</e2servicereference>";
			os << "<e2isplayable>" << res  << "</e2isplayable>";
			os << "</e2serviceplayable>";
		} else if (uri == "getservices") {
			if (query.empty()) {
				const ServiceList& l = TitanAdapter::getAdapter()->getServices();

				os << xml << endl;
				os << l;
			} else if (query.find("sRef=") == 0) {
				string serviceRef = Util::getTitanRef(query.substr(5));
				const ServiceList& l = TitanAdapter::getAdapter()->getServices(serviceRef);

				os << xml << endl;
				os << l;
			} else {
				notFound = true;
			}
		} else if (uri == "getlocations" || uri == "getcurrlocation") {
			// add dummy location
			os << xml << endl;
			os << "<e2locations>";
			os << "<e2location>/media/hdd/movie</e2location>" << endl;
			os << "</e2locations>";
		} else if (uri == "movielist") {
			list<Movie> movies = TitanAdapter::getAdapter()->getMovies();
			os << xml << endl;
			os << movies;
		} else if ((uri == "moviedelete") && query.find("sRef=") == 0) {
			string ref = Util::getTitanRef(query.substr(5));
			bool deleted = TitanAdapter::getAdapter()->deleteMovie(ref);
			os << "<e2simplexmlresult>";
			os << "<e2state>" << deleted << "</e2state>" << endl;
			os << "</e2simplexmlresult>";
		} else if ((uri == "epgservicenow") && query.find("sRef=") == 0) {
			string serviceRef = Util::getTitanRef(query.substr(5));
			EventList l = TitanAdapter::getAdapter()->getEpgNow(serviceRef);

			os << xml << endl;
			os << l;
		} else if ((uri == "epgservicenext") && query.find("sRef=") == 0) {
			string serviceRef = Util::getTitanRef(query.substr(5));
			EventList l = TitanAdapter::getAdapter()->getEpgNext(serviceRef);

			os << xml << endl;
			os << l;
		} else if ((uri == "epgservice") && query.find("sRef=") == 0) {
			string serviceRef = Util::getTitanRef(query.substr(5));
			EventList l = TitanAdapter::getAdapter()->getEpg(serviceRef);

			os << xml << endl;
			os << l;
		} else if (uri == "powerstate" && query.empty()) {	
			// TODO: response
			os << xml << endl;
			os << "<e2powerstate><e2instandby>";
		
			if (TitanAdapter::getAdapter()->isRunning()) {
				os << "true";
			} else {
				os << "false";
			}

			os << "</e2instandby></e2powerstate>" << endl;
		} else if (uri == "powerstate" && query.find("newstate=") == 0) {
			int state = Util::getInt(query.substr(9).c_str());

			TitanAdapter::getAdapter()->setPowerState((PowerState) state);

			// TODO: response
			os << xml << endl;
			os << "<e2powerstate><e2instandby>";
		
			if (state == PS_STANDBY || state == PS_DEEP_STANDBY) {
				os << "true";
			} else {
				os << "false";
			}

			os << "</e2instandby></e2powerstate>" << endl;
		} else if (uri == "remotecontrol" && query.find("command=") == 0) {
			string rc = query.substr(8);
			int code = Util::getInt(rc);
			bool res = TitanAdapter::getAdapter()->sendRc(code);

			os << xml << endl;
			os << "<e2remotecontrol>";
			os << "<e2result>" << res << "</e2result>" << endl;
			os << "</e2remotecontrol>";
		} else if (uri == "epgsimilar" && query.find("sRef=") == 0) {
			EventList l;
			os << xml << endl;
			os << l;
		} else if ((uri == "epgnow" || uri == "epgnext") && query.find("bRef=") == 0) {
			EventList l; // TODO:
			os << xml << endl;
			os << l;
		} else if (uri == "vol") { // gets or sets volume
			Volume vol;
			
			if (query.find("set=") == 0) {
				string val = query.substr(4);

				if (val == "mute") {
					TitanAdapter::getAdapter()->setMute(true, vol);
				} else if (val.find("set") == 0) {
					TitanAdapter::getAdapter()->setVolume(Util::getInt(val.substr(3)), vol);
				} else {
					TitanAdapter::getAdapter()->setMute(false, vol);
				}
			} else {
				TitanAdapter::getAdapter()->getVolume(vol);
			}
			
			os << xml << endl;
			os << vol;
		} else if (uri == "about") {
			DeviceInfo device;
			Channel channel;

			TitanAdapter::getAdapter()->about(device, channel);
			os << xml << endl;
			TitanAdapter::about(os, device, channel);
		} else if (uri == "deviceinfo") {
			DeviceInfo device;

			TitanAdapter::getAdapter()->deviceInfo(device);

			os << xml << endl;
			os << device;
		} else {
			notFound = true;
		}
	} else if ((uri == "/file" || uri == "/file/") && query.find("file=") == 0) {
		string ref = Util::getTitanRef(query.substr(5));
		ref = Util::urlEncode(ref);

		ostringstream response;
		string server = movedTemporarily(conn, response);

		if (!server.empty()) {
			response << "Location: http://" <<  server << ":" <<  Config::getTitanDataPort() << "/0%2c0%2c/media/hdd/movie/" << ref << "\r\n\r\n";
		}

		string resp = response.str();
		mg_write(conn, resp.c_str(), resp.length());

		return;
	} else if (uri.find("/1:0:1:") == 0) {
		uri = "/" +  Util::urlEncode(Util::getTitanRef(uri.substr(1)));
	
		ostringstream response;
		string server = movedTemporarily(conn, response);
		
		if (!server.empty()) {
			response << "Location: http://" << server << ":" << Config::getTitanDataPort() << uri << "\r\n\r\n";
		}

		string resp = response.str();
		mg_write(conn, resp.c_str(), resp.length());

		return;
	} else {
		notFound = true;
	}

	ostringstream response;

	response << "HTTP/1.1 ";

	if (notFound) {
		response << "404 Not Found\r\n";
	} else {
		response << "200 OK\r\n";
	}

	
	string data = os.str();
	int len = data.length();

	if (notFound) {
		response << "Date: " << Util::getHttpDate() << "\r\n";
		response << "Connection: close\r\n";
		response << "Server: GsE2TitanBridge/1.0\r\n";
	} else {
		//response << "Transfer-Encoding: chunked\r\n";
		response << "Date: " << Util::getHttpDate() << "\r\n";
		response << "Content-Type: text/xml; charset=UTF-8\r\n";
		response << "Content-Length: " << len << "\r\n";
		response << "Server: GsE2TitanBridge/1.0\r\n";
	}

	response << "\r\n";

	string headers = response.str();

	mg_write(conn, headers.c_str(), headers.length());

	if (!notFound) {
		
//		writeChunk(conn, data);
		mg_write(conn, data.c_str(), data.length());
	}
}


int requestHandler(struct mg_connection* conn) {
	const struct mg_request_info *request = mg_get_request_info(conn);

	string uri = request->uri;
	string query;

	if (request->query_string) {
		char buf[512];
		buf[0] = 0;
		Util::urlDecode(request->query_string, strlen(request->query_string), buf, 512);
		query = buf;
	}

	handle(conn, uri, query);

	return 1;
}


/*int proxyRequestHandler(struct mg_connection* conn) {
	const struct mg_request_info *request = mg_get_request_info(conn);

	string uri = request->uri;
	// TODO: log
	// cout << "proxy: " << uri << endl;

	if (uri.find("/1:0:1:") == 0) {
		uri = "/" + Util::getTitanRef(uri.substr(1));
	}

	
	char buf[512];
	buf[0] = 0;
	Util::urlEncode(uri.c_str(), buf, 512);
	uri = buf;
	
	ostringstream response;
	const char* host = mg_get_header(conn, "Host");

	if (host) {
		response << "HTTP/1.1 302 Moved Temporarily\r\n";
	} else {
		response << "HTTP/1.1 400 Bad Request\r\n";
	}

	response << "Date: " << Util::getHttpDate() << "\r\n";

	if (host) {
		string server(host);

		string::size_type pos = server.find(':');

		if (pos != string::npos) {
			server = server.substr(0, pos);
		}

		const string& titanHost = Config::getTitanHost();

		if (titanHost != "127.0.0.1") { // we are not local -> use configured titan host
			server = titanHost.c_str();
		}

		response << "Location: http://" << server << ":" << Config::getTitanDataPort() << uri << "\r\n\r\n";
	}

	string resp = response.str();
	mg_write(conn, resp.c_str(), resp.length());

	return 1;
}*/

void startAsDaemon() {
#ifndef _WIN32
	if (getppid() == 1) { // already daemon
		return;
	}

	pid_t p = fork();

	if (p < 0) {
		cerr << "fork failed: " << errno << endl;
		exit(errno);
	} else if (p > 0) { // parent process
		exit(0);
	} 

	// child

	pid_t sid = setsid();

	if (sid < 0) {
		cerr << "setsid failed: " << errno << endl;
		exit(errno);
	}

	chdir("/");
	umask(0);

	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
#endif
}

int main(int argc, char** argv) {

	if (!Config::parse(argc, (const char**) argv)) {
		return -1;
	}

	if (Config::isDaemon()) {
		startAsDaemon();
	}

	TitanAdapter adapter;
	adapter.init();

	string threads = Util::valueOf(Config::getThreads());
	string port = Util::valueOf(Config::getPort()) + "," + Util::valueOf(Config::getDataPort());

	struct mg_context *ctx;
	struct mg_callbacks callbacks;
	const char *options[] = {"listening_ports", port.c_str(), "num_threads", threads.c_str(), 0};

	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = requestHandler;
	ctx = mg_start(&callbacks, 0, options);


	/*struct mg_context *proxyCtx = 0;
	struct mg_callbacks proxyCallbacks;
	string dataPort = Util::valueOf(Config::getDataPort());
	const char *proxyOptions[] = {"listening_ports", dataPort.c_str(), "num_threads", threads.c_str(), 0};


	memset(&proxyCallbacks, 0, sizeof(proxyCallbacks));

	proxyCallbacks.begin_request = proxyRequestHandler;

	proxyCtx = mg_start(&proxyCallbacks, 0, proxyOptions);*/

	if (!ctx) {
		cerr << "Server could not be started" << endl;
		return -1;
	}

	if (!Config::isDaemon()) {
		cin.ignore();
		mg_stop(ctx);
	} else {
#ifndef _WIN32
		pause();
#endif
	}

	/*if (proxyCtx) {
		mg_stop(proxyCtx);
	}*/

	return 0;
}
