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
#include "Log.h"

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

Reference getRef(const string& e2Ref) {
	Reference ref;
	istringstream is(e2Ref);

	is >> ref;

	return ref;
}

string getTitanRef(const Reference& ref) {
	ostringstream os;

	unsigned int tid = (ref.nid << 16) + ref.tid;
	os << ref.sid << ',' << tid;

	return os.str();
}

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

void playList(ostream& os, const ServiceList& services, const string& server) {
	os << "#EXTM3U" << endl;
	os << "#EXTVLCOPT--http-reconnect=true" << endl;

	for (ServiceList::const_iterator it = services.begin(); it != services.end(); ++it) {
		const Service& s = *it;

		if (!s.bouquet) {
			ostringstream ref;
			ref << s.ref;

			os << "#EXTINF:-1," << s.name.c_str() << endl;
			os << "http://" << server  << "/" << Util::urlEncode(ref.str()) << endl;
		}
	}
}

Timer getTimerData(const map<string, string>& params) {
	Timer t;

	time_t begin = Util::getTime(params, "begin");

	if (begin > 0) {
		t.begin = begin;
		t.end = Util::getTime(params, "end");
		t.name = Util::getString(params, "name");
		t.repeat = Util::getInt(params, "repeated");
		t.service = getRef(Util::getString(params, "sRef"));

	} else {
		Reference ref =  getRef(Util::getString(params, "sRef"));
		string titanRef = getTitanRef(ref);
		unsigned int eventId = Util::getInt(params, "eventid");

		Event e;
		if (TitanAdapter::getAdapter()->getEpg(titanRef, eventId, e)) {
			t.begin = e.start;
			t.end = e.start + e.dur;
			t.name = e.title;
			t.serviceName = e.servicveName;
			t.service = ref;
		}
	}

	t.id = Util::getString(params, "eit", "");
	t.afterEvent = (AfterEvent) Util::getInt(params, "afterevent", 3); // auto
	t.justPlay = Util::getInt(params, "justplay");

	return t;
}

bool servicePlayAble(const Reference& ref) {
	Reference current = TitanAdapter::getAdapter()->getCurrentReference();

	return ((ref.tid == current.tid) && (ref.nid == current.nid));
}

bool servicePlayAble(const Reference& ref, const Reference& playing) {
	Reference current = TitanAdapter::getAdapter()->getCurrentReference();

	if (current == playing) {
		return true;
	}

	return ((ref.tid == current.tid) && (ref.nid == current.nid));
}

void checkAutoZap(const Reference& ref) {
	if (Config::isAutoZap()) {
		if (!servicePlayAble(ref)) {
			string titanRef = getTitanRef(ref);

			TitanAdapter::getAdapter()->zap(titanRef);
		}
	}
}

void handle(struct mg_connection* conn, std::string& uri, const string& query, const map<string, string>& param) {
	static const string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	static const string nl = "\r\n";
	static const string defaultContentType = "Content-Type: text/xml; charset=UTF-8\r\n";
	static const string m3uContentType = "Content-Type: application/vnd.apple.mpegurl\r\nContent-Disposition: attachmant; filename=playlist.m3u8\r\n";

	string contentType = defaultContentType;

	string msg = "GET " + uri;

	if (!query.empty()) {
		msg.append(", q=").append(query);
	}

	Log::getLogger()->log(Log::INFO, "SRV", msg);

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
			string sref = Util::getString(param, "sRef", "");
			string playing = Util::getString(param, "sRefPlaying", "");

			bool res = servicePlayAble(getRef(sref), getRef(playing));

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
		} else if (uri == "timerlist") {
			TimerList l = TitanAdapter::getAdapter()->getTimers();

			os << xml << endl;
			os << l;
		} else if (uri == "timeraddbyeventid" || uri == "timeradd" || uri == "timerdelete" || uri == "timerchange") {

			Timer t = getTimerData(param);
			bool res;
			
			if (uri == "timerdelete") {
				res = TitanAdapter::getAdapter()->deleteTimer(t); 
			} else if (uri == "timerchange") {
				res = TitanAdapter::getAdapter()->changeTimer(t);
			} else {
				res = TitanAdapter::getAdapter()->addTimer(t);
			}

			os << xml << endl;
			os << "<e2simplexmlresult>";
			os << "<e2state>" << (res ? "True" : "False") << "</e2state>";
			os << "<e2statetext>" << (res ? "Ok": "Not ok") << "</e2statetext>";
			os << "</e2simplexmlresult>";

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
		} else if ((uri == "epgnow") && query.find("bRef=") == 0) {
			string ref = Util::getTitanRef(query.substr(5));
			EventList l = TitanAdapter::getAdapter()->getEpgNowForBouquet(ref);
			os << xml << endl;
			os << l;
		} else if ((uri == "epgnext") && query.find("bRef=") == 0) {
			string ref = Util::getTitanRef(query.substr(5));
			EventList l = TitanAdapter::getAdapter()->getEpgNextForBouquet(ref);
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
		} else if (uri == "services.m3u") {
			string ref = Util::getTitanRef(Util::getString(param, "bRef", ""));
			ServiceList services = TitanAdapter::getAdapter()->getServices(ref);
			const char* host = mg_get_header(conn, "Host");
			string server = host;

			playList(os, services, host);
			contentType = m3uContentType;
		} else {
			notFound = true;
		}
	} else if ((uri == "/file" || uri == "/file/") && query.find("file=") == 0) {
		string ref = Util::getTitanRef(query.substr(5));
		ref = Util::urlEncode(ref);

		ostringstream response;
		string server = movedTemporarily(conn, response);

		if (!server.empty()) {
			response << "Location: http://" <<  server << ":" <<  Config::getTitanDataPort() << "/0%2c0%2c/media/hdd/movie/" << ref << nl << nl;
		}

		Log::getLogger()->log(Log::INFO, "SRV", "302 Moved Temporarily");

		string resp = response.str();
		mg_write(conn, resp.c_str(), resp.length());

		return;
	} else if (uri.find("/1:0:1:") == 0) {
		Reference e2Ref = getRef(uri.substr(1));

		uri = "/" +  Util::urlEncode(Util::getTitanRef(uri.substr(1)));
	
		ostringstream response;
		string server = movedTemporarily(conn, response);
	
		checkAutoZap(e2Ref); // we check here if we should automatically zap to the new channel
		
		if (!server.empty()) {
			response << "Location: http://" << server << ":" << Config::getTitanDataPort() << uri << nl << nl;
		}

		Log::getLogger()->log(Log::INFO, "SRV", "302 Moved Temporarily");

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
		Log::getLogger()->log(Log::INFO, "SRV", "404 Not Found");
		response << "Date: " << Util::getHttpDate() << nl;
		response << "Connection: close\r\n";
		response << "Server: " << Version::getVersion() << nl;
	} else {
		Log::getLogger()->log(Log::INFO, "SRV", "200 OK, len=" + Util::valueOf(len));

		response << "Date: " << Util::getHttpDate() << nl;
		response << contentType;
		response << "Content-Length: " << len << nl;
		response << "Server: " << Version::getVersion() << nl;
	}

	response << nl;

	string headers = response.str();

	mg_write(conn, headers.c_str(), headers.length());

	if (!notFound) {
		mg_write(conn, data.c_str(), data.length());
	}
}


int requestHandler(struct mg_connection* conn) {
	const struct mg_request_info *request = mg_get_request_info(conn);

	string uri = request->uri;
	string query;
	map<string, string> queryParam;

	if (request->query_string) {
		query = request->query_string;
		
		queryParam = Util::parseQuery(query);

		char buf[512];
		buf[0] = 0;
		Util::urlDecode(request->query_string, strlen(request->query_string), buf, 512);
		query = buf;
	}

	Log::getLogger()->log(Log::DEBUG, "SRV", "new request");
	handle(conn, uri, query, queryParam);
	Log::getLogger()->log(Log::DEBUG, "SRV", "request handled");

	return 1;
}

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

	Log log(Config::getLogFile(), Config::getMaxLogFileSize() * 1024);
	Log::setLogger(&log);

	log.log(Log::INFO, "SRV", "start server: " + Version::getVersion());

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

	if (!ctx) {
		string msg("Server could not be started (please check '/etc/inetd.conf' if port 8001 is not used)");
		log.log(Log::ERROR, "SRV", msg);
		cerr << msg << endl;

		return -1;
	}

	log.log(Log::INFO, "SRV", "running");

	if (!Config::isDaemon()) {
		cin.ignore();
		mg_stop(ctx);
	} else {
#ifndef _WIN32
		pause();
#endif
	}

	return 0;
}
