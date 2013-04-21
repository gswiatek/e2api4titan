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
// this software without specific prior written permission.tr
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

#include "TitanAdapter.h"
#include "FileHelper.h"
#include "Config.h"
#include "Client.h"
#include "Util.h"
#include "Log.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>
#endif

using namespace gs::e2;
using namespace std;

TitanAdapter* TitanAdapter::m_adapter = 0;

TitanAdapter::TitanAdapter(): m_channelReader(m_providerReader, m_transponderReader), m_serviceReader(m_channelReader, true) {
	m_rcCodes[352] = "rcok";
	m_rcCodes[174] = "rcexit";
	m_rcCodes[388] = "rctext";
	m_rcCodes[365] = "rcepg";
	m_rcCodes[167] = "rcrecord";
	m_rcCodes[103] = "rcup";
	m_rcCodes[108] = "rcdown";
	m_rcCodes[105] = "rcleft";
	m_rcCodes[106] = "rcright";
	m_rcCodes[139] = "rcmenu";
	m_rcCodes[208] = "rcff";
	m_rcCodes[168] = "rcfr";
	m_rcCodes[207] = "rcplay";
	m_rcCodes[119] = "rcpause";
	m_rcCodes[128] = "rcstop";
	m_rcCodes[115] = "rcvolup";
	m_rcCodes[114] = "rcvoldown";
	m_rcCodes[402] = "rcchup"; // 104
	m_rcCodes[403] = "rcchdown"; // 109
	m_rcCodes[113] = "rcmute";
	m_rcCodes[398] = "rcred";
	m_rcCodes[399] = "rcgreen";
	m_rcCodes[401] = "rcblue";
	m_rcCodes[400] = "rcyellow";
	m_rcCodes[358] = "rcinfo"; // 138
	m_rcCodes[396] = "rcrecall";
	m_rcCodes[371] = "rcvformat";
	m_rcCodes[372] = "rcsubtitel";
	m_rcCodes[3113] = "rcresolution";
	m_rcCodes[377] = "rctvradio"; // TV, 373
	m_rcCodes[385] = "rctvradio"; // Radio
	m_rcCodes[409] = "rcsleep";
	m_rcCodes[394] = "rcmedia";
	m_rcCodes[364] = "rcfav";
	m_rcCodes[353] = "rccheck";
	m_rcCodes[407] = "rcnext";
	m_rcCodes[138] = "rchelp";
	m_rcCodes[412] = "rcprev";
	m_rcCodes[357] = "rchbbtv";
	m_rcCodes[354] = "rczoom";
	m_rcCodes[116] = "rcpower";
	m_rcCodes[3401] = "rcsubchannel";
	m_rcCodes[370] = "rcpiplist";
	m_rcCodes[2] = "rc1";
	m_rcCodes[3] = "rc2";
	m_rcCodes[4] = "rc3";
	m_rcCodes[5] = "rc4";
	m_rcCodes[6] = "rc5";
	m_rcCodes[7] = "rc6";
	m_rcCodes[8] = "rc7";
	m_rcCodes[9] = "rc8";
	m_rcCodes[10] = "rc9";
	m_rcCodes[11] = "rc0";
	m_rcCodes[414] = "rcpup"; // 414
	m_rcCodes[415] = "rcpdown"; // 415
}

TitanAdapter::~TitanAdapter() {

}

void TitanAdapter::init() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "init");
	m_adapter = this; // store the reference

	readInfo();

	readTransponders();
	readProviders();
	readChannels();
	readBouquets();

	m_channelReader.cleanup(); // we don't need the channel information anymore
	m_transponderReader.cleanup();
}

TitanAdapter* TitanAdapter::getAdapter() {
	return m_adapter;
}

std::string TitanAdapter::getRcName(int code) {
	map<int, string>::const_iterator it = m_rcCodes.find(code);

	string res;

	if (it != m_rcCodes.end()) {
		res = it->second;
	}

	return res;
}

const ServiceList& TitanAdapter::getServices() const {
	return m_serviceReader.getServices();
}

const ServiceList& TitanAdapter::getServices(const std::string& reference) const {
	const string& fileName = m_serviceReader.getBouquetFile(reference);

	if (!fileName.empty()) {
		return m_serviceReader.getServices(fileName);
	} else {
		return m_serviceReader.getServices(reference);
	}
}

void TitanAdapter::about(DeviceInfo& device, Channel& currentService) {
	device = m_info;

}

void TitanAdapter::deviceInfo(DeviceInfo& device) {
	device = m_info;
}

std::list<std::string> TitanAdapter::getLocations() {
	list<string> l;

	return l;
}

void TitanAdapter::readInfo() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "readInfo");

#ifndef _WIN32
	const char* frontendNames[] = {"A", "B", "C", "D", 0};
	int num = 0; 
	char name[30];

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (num == 4) {
				break;
			}

			snprintf(name, 30, "/dev/dvb/adapter%d/frontend%d", i, j);

			int fd = open(name, O_RDONLY);

			if (fd >= 0) {
				struct dvb_frontend_info info;

				if (ioctl(fd, FE_GET_INFO, &info) >= 0) {
					Frontend f;

					string fn = frontendNames[num++];
					if (info.type == 0) {
						fn.append(" (DVB-S)");
					} else if (info.type == 1) {
						fn.append(" (DVB-C)");
					} else if (info.type == 2) {
						fn.append(" (DVB-T)");
					}

					f.name = fn;
					f.model = info.name;

					m_info.frontends.push_back(f);
				}

				close(fd);
			}
		}
	}

#endif

	m_info.webIfVer = "1.6.8";

	ifstream in;

	string modelFile = Config::getEtcDir() + "model";
	in.open(modelFile.c_str());

	if (in.is_open()) {
		if (in.good()) {
			in >> m_info.name;
		}

		in.close();
	} else {
		Log::getLogger()->log(Log::ERROR, "ADP", "could not read file: " + modelFile);
	}

	string versionFile = Config::getEtcDir() + "version";
	in.open(versionFile.c_str());

	if (in.is_open()) {
		if (in.good()) {
			in >> m_info.ver;
		}

		in.close();
	}

	versionFile = Config::getEtcDir() + "image-version";
	in.open(versionFile.c_str());

	if (in.is_open()) {
		if (in.good()) {
			in >> m_info.imageVer;

			if (m_info.imageVer.find("version=") == 0) {
				m_info.imageVer = m_info.imageVer.substr(8);
			}
		}

		in.close();
	}
}

void TitanAdapter::readTransponders() {
	string config = Config::getTitanDir() + "transponder";

	FileHelper::readConfigFile(config, m_transponderReader);
}

void TitanAdapter::readBouquets() {
	string config = Config::getTitanDir() + "bouquets.cfg";

	FileHelper::readConfigFile(config, m_serviceReader);
}

void TitanAdapter::readChannels() {
	string config = Config::getTitanDir() + "channel";

	FileHelper::readConfigFile(config, m_channelReader);
}

void TitanAdapter::readProviders() {
	string config = Config::getTitanDir() + "provider";

	FileHelper::readConfigFile(config, m_providerReader);
}

bool TitanAdapter::zap(const string& serviceRef) {
	string reply;

	string id = serviceRef;
	replace(id.begin(), id.end(), ',', '&');

	return Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?switch&" + id, reply);
}

EventList TitanAdapter::getEpg(const string& serviceRef) {
	EventList res;

	string id = serviceRef;

	replace(id.begin(), id.end(), ',', '&');

	string reply;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getsingleepg&" + id, reply)) {
		EpgReader reader;

		istringstream is(reply);

		FileHelper::handleStream(is, reader);

		res = reader.getEvents();

		if (!res.empty()) {
			Reference ref; // the reference
			string name; // the name of the channel

			if (m_channelReader.lookup(serviceRef, name, ref)) {
				for (EventList::iterator it = res.begin(); it != res.end(); ++it) {
					it->service = ref;
					it->servicveName = name;
				}
			}
		}
	}

	return res;
}

bool TitanAdapter::deleteMovie(const string& ref) {
	string reply;

	// TODO: movie location configurable
	return Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?delmoviefile&/media/hdd/movie&1&" + Util::urlEncode(ref, true), reply);
}

MovieList TitanAdapter::getMovies() {
	list<string> files;

	string reply;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getmoviefilelist", reply)) {
		MovieReader reader;

		istringstream is(reply);

		FileHelper::handleStream(is, reader);

		files =  reader.getMovies();
	}

	list<Movie> movies;

	for (list<string>::const_iterator it = files.begin(); it != files.end(); ++it) {
		const string& name = *it;

		Movie m;
		m.fileName = name;
		m.ref.path = name;

		string::size_type pos = name.find('-');

		if (pos != string::npos) {
			m.serviceName = name.substr(0, pos);

			string::size_type pos2 = name.rfind('-');

			if (pos2 > pos + 1) {
				m.title = name.substr(pos + 1, pos2 - pos - 1);

				m.recTime = Util::parseTime(name.substr(pos2 + 1));
			}
		}

		movies.push_back(m);
	}

	return movies;
}

bool TitanAdapter::isRunning() {
	bool res = false;
	string reply;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?boxstatus", reply)) {
		res = (reply == "running") ;
	}

	return res;
}

bool TitanAdapter::sendRc(int code) {
	string rc = getRcName(code);

	if (rc.empty()) {
		return false;
	}

	string reply;

	Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?sendrc&" + rc, reply);

	return true;
}

bool TitanAdapter::setPowerState(PowerState state) {

	bool res = false;
	string reply;

	if (state == PS_DEEP_STANDBY) {
		res = Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?poweroff", reply);		
	} else if (state == PS_REBOOT) {
		res = Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?restart", reply);
	} else if (state == PS_STANDBY) {
		res = Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?standby", reply);
	} else if (state == PS_TOOGLE_STANDBY) {
		if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?boxstatus", reply)) {
			if (reply == "running") {
				res = Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?standby", reply);
			} else {
				res = Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?sendrc&rcpower", reply);
			}
		}
	} else if (state == PS_WAKEUP) {
		res = Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?sendrc&rcpower", reply);
	}

	return res;
}

bool TitanAdapter::getEpg(const string& ref, unsigned int eventId, Event& event) {
	string q = ref + "&" + Util::valueOf(eventId);

	return getEpg(q, event);
}

bool TitanAdapter::getEpg(const string& ref, Event& event) {
	string q = ref;

	replace(q.begin(), q.end(), ',', '&');

	string reply;
	bool res = Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getepg&" + q, reply);

	if (res) {
		vector<string> values = Util::getTokens(reply);

		if (values.size() < 5) {
			return false;
		}

		event.title  = values[0];
		event.start = Util::getTime(values[1]);
		event.dur = Util::getTime(values[2]) - event.start;
		event.desc = values[3];
		event.extended = values[4];
	}

	return res;
}

EventList TitanAdapter::getEpgNow(const string& ref) {

	EventList res;
	TitanChannel current;

	if (getActive(current)) {
		if (current.eventId > 0) {
			Event e;

			if (getEpg(ref + "&" + Util::valueOf(current.eventId), e)) {
				e.id = current.eventId;
				res.push_back(e);
			}
		}
	}

	return res;
}

EventList TitanAdapter::getEpgNext(const string& ref) {
	EventList res;

	TitanChannel current;

	if (getActive(current)) {
		if (current.nextId > 0) { // when next id is set then get the whole EPG info
			Event e;

			if (getEpg(ref + "&" + Util::valueOf(current.nextId), e)) {
				e.id = current.eventId;
				res.push_back(e);
			}
		} else if (current.eventId > 0) { // when current event id is set then get only the short info if available
			Event e;

			e.title = current.nextDesc;
			e.desc = current.nextDesc;
			e.start = current.nextStart;
			e.dur = (current.nextStop - current.nextStart);

			res.push_back(e);
		}
	}

	return res;
}


bool TitanAdapter::getActive(TitanChannel& current) {
	string reply;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getaktservice", reply)) {
		vector<string> values = Util::getTokens(reply);

		if (values.size() >= 12) {
			int pos = 0;

			current.channelName = values[pos++];
			current.proc = Util::getInt(values[pos++]);
			current.serviceId = Util::getUInt(values[pos++]);
			current.transponderId = Util::getUInt(values[pos++]);
			current.channelList = values[pos++];
			current.serviceType = Util::getInt(values[pos++]);
			current.eventId = Util::getUInt(values[pos++]);

			if (current.eventId > 0) {
				current.actStart = Util::getTime(values[pos++]);
				current.actDesc = values[pos++];
				current.nextStart = Util::getTime(values[pos++]);
				current.nextStop = Util::getTime(values[pos++]);
				current.nextDesc = values[pos];
			}

			if (values.size() > 12) {
				current.nextId = Util::getUInt(values[12]);
			}

			return true;
		}
	}

	return false;
}

Reference TitanAdapter::getCurrentReference() {
	CurrentService service = getCurrent(false);

	return service.service.ref;
}

CurrentService TitanAdapter::getCurrent(bool withEpg) {

	CurrentService res;
	TitanChannel titanCurrent;

	if (getActive(titanCurrent)) {
		res.service.bouquet = false;
		res.service.name = titanCurrent.channelName;
		res.service.ref.type = RT_DVB;
		res.service.ref.sid = titanCurrent.serviceId;
		res.service.ref.tid = titanCurrent.transponderId & 0x0ffff;
		res.service.ref.nid = (titanCurrent.transponderId >> 16) & 0x0ffff;
		
		// TODO: channel lookup, because of DVB namespace

		if (withEpg && titanCurrent.eventId > 0) {
			Event e;
				
			if (getEpg(Util::valueOf(titanCurrent.serviceId) + "&" + Util::valueOf(titanCurrent.transponderId) + "&" + Util::valueOf(titanCurrent.eventId), e)) {
				e.id = titanCurrent.eventId;
				res.events.push_back(e);				
			}
		}
	}

	return res;
}

EpgReader::EpgReader() {

}

EpgReader::~EpgReader() {

}

const EventList& EpgReader::getEvents() const {
	return m_events;
}

void EpgReader::handleLine(const vector<char*>& line) {
	if (line.size() < 6) {
		return;
	}

	Event e;
	e.title = line[0];
	e.start = Util::getTime(line[1]);
	e.dur = Util::getTime(line[2]) - e.start;
	e.desc = line[3];
	e.extended = line[4];
	e.id = Util::getUInt(line[5]);

	m_events.push_back(e);
}

void EpgReader::finished() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "events=" + Util::valueOf(m_events.size()));
}

MovieReader::MovieReader() {

}

MovieReader::~MovieReader() {

}

const std::list<std::string>& MovieReader::getMovies() const {
	return m_movies;
}

void MovieReader::handleLine(const vector<char*>& line) {
	if (line.size() < 2) {
		return;
	}

	string name = line[0];
	string type = line[1];
	
	if (type == "1") {
		m_movies.push_back(name);
	}
}

void MovieReader::finished() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "movies=" + Util::valueOf(m_movies.size()));
}

ServiceReader::ServiceReader(ChannelReader& channelReader, bool bouquet): m_channelReader(channelReader), m_bouquetFile(bouquet) {

}

ServiceReader::~ServiceReader() {
	map<string, ServiceReader*>::iterator it = m_readers.begin();

	while (it != m_readers.end()) {
		delete it->second;
		++it;
	}

	m_readers.clear();
}

void ServiceReader::handleLine(const vector<char*>& line) {
	if (m_bouquetFile) {
		if (line.size() < 3) {
			return;
		}

		string loc(line[2]);
		
		Util::trim(loc);

#ifndef _WIN32 
		if (!loc.empty() && loc[0] != '/') { // when location is relative path then we prepend the configured titan dir
			loc = Config::getTitanDir() + loc;
		}
#endif

		string fileName(loc);
		
		string::size_type pos = fileName.rfind('/');

		if (pos != string::npos) {
			fileName = fileName.substr(pos + 1);
		}

		Service s;

		s.bouquet = true;
		s.name = line[0];
		s.ref.type = RT_DVB;
		s.ref.flags = 7;
		s.ref.tv_radio = 1;
		s.ref.path = fileName;

		m_services.push_back(s);
		m_bouquetName[fileName] = s.name;
		m_bouquetFileName[s.name] = fileName;

		ServiceReader* sr = new ServiceReader(m_channelReader, false);

		Log::getLogger()->log(Log::INFO, "ADP", "read bouquet: " + loc);
		FileHelper::readConfigFile(loc, *sr);

		m_readers[fileName] = sr;
	} else {
		if (line.size() < 2) {
			return;
		}

		unsigned int sid = Util::getUInt(line[0]);
		unsigned int tid = Util::getUInt(line[1]);
		string ref = string(line[0]) + "," + line[1];
		Util::trim(ref);
		Service s;

		s.bouquet = false;
		s.ref.type = RT_DVB;
		s.ref.tv_radio = 1;
		s.ref.sid = sid;
		s.ref.tid = tid & 0x0ffff;
		s.ref.nid = (tid >> 16) & 0x0ffff;

		Channel channel;

		if (m_channelReader.lookup(ref, channel)) {
			s.name = channel.serv.name;
			s.ref.dvbName = channel.serv.ref.dvbName;
		} else {
			s.name = ref;
		}

		m_services.push_back(s);
	}

}

void ServiceReader::finished() {
	if (m_bouquetFile) {
		Log::getLogger()->log(Log::DEBUG, "ADP", "bouquets=" + Util::valueOf(m_services.size()));
	} else {
		Log::getLogger()->log(Log::DEBUG, "ADP", "services=" + Util::valueOf(m_services.size()));
	}
}

const ServiceList& ServiceReader::getServices() const {
	return m_services;
}

const ServiceList& ServiceReader::getServices(const string& bouquet) const {

	map<string, ServiceReader*>::const_iterator it = m_readers.find(bouquet);

	if (it != m_readers.end()) {
		return it->second->getServices();
	} else {
		Log::getLogger()->log(Log::ERROR, "ADP", "bouquet not found: '" + bouquet + "'");

		if (bouquet == "bouquets.tv") { // return all bouquets
			return getServices();
		}

		return m_empty;
	}
}

const string& ServiceReader::getBouquetName(const string& ref) const {
	static string empty;

	map<string, string>::const_iterator it = m_bouquetName.find(ref);

	if (it != m_bouquetName.end()) {
		return it->second;
	}

	return empty;
}

const string& ServiceReader::getBouquetFile(const string& name) const {
	static string empty;

	map<string, string>::const_iterator it = m_bouquetFileName.find(name);

	if (it != m_bouquetFileName.end()) {
		return it->second;
	}

	return empty;
}

ChannelReader::ChannelReader(ProviderReader& providerReader, TransponderReader& transponderReader): m_providerReader(providerReader), m_transponderReader(transponderReader) {

}

ChannelReader::~ChannelReader() {
	cleanup();
}

void ChannelReader::cleanup() {
	map<string, Channel*>::iterator it = m_channels.begin();

	while (it != m_channels.end()) {
		delete it->second;
		++it;
	}

	m_channels.clear();	
}

void ChannelReader::handleLine(const vector<char*>& line) {
	if (line.size() < 11) {
		return;
	}

	Channel* c = new Channel();

	unsigned int tid = Util::getUInt(line[1]);

	c->serv.name = line[0];
	c->serv.ref.type = RT_DVB;
	c->serv.ref.tid = tid & 0x0ffff;
	c->serv.ref.nid = (tid >> 16) & 0x0ffff;

	Transponder t;

	if (m_transponderReader.lookup(tid, t)) {
		c->serv.ref.dvbName = getDvbNamespace(c->serv.ref.nid, c->serv.ref.tid, t.orbitPos, t.freq, t.pol);
	}

	c->prov = m_providerReader.lookup(Util::getUInt(line[2]));
	c->serv.ref.sid = Util::getUInt(line[3]);
	c->vpid = Util::getUInt(line[8]);
	c->apid = Util::getUInt(line[9]);

	string id = string(line[3]) + "," + line[1];
	Util::trim(id);
	m_channels[id] = c;
}


void ChannelReader::finished() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "channels=" + Util::valueOf(m_channels.size()));
}


bool ChannelReader::lookup(const string& id, Channel& channel) {
	bool res = false;
	map<string, Channel*>::const_iterator it = m_channels.find(id);

	if (it != m_channels.end()) {
		res = true;
		channel = *it->second;

		m_usedChannels[id] = channel.serv; // store the used channel
	} else {
		Log::getLogger()->log(Log::ERROR, "ADP", "channel not found: '" + id + "'");
	}

	return res;
}

bool ChannelReader::lookup(const string&id, string& name, Reference& ref) const {
	bool res = false;
	map<string, Service>::const_iterator it = m_usedChannels.find(id);

	if (it != m_usedChannels.end()) {
		const Service& service = it->second;

		res = true;
		name = service.name;
		ref = service.ref;
	}

	return res;
}


Event ChannelReader::getEventFromChannel(const TitanChannel& channel, bool next) const {
	Event res;

	res.current = time(0);
	res.desc = (next ? channel.nextDesc : channel.actDesc);
	res.title = res.desc;
	res.servicveName = channel.channelName;
	res.start = (next ? channel.nextStart : channel.actStart);
	res.dur = (next ? (channel.nextStop - res.start) : (channel.nextStart - res.start));
	res.id = (next ? channel.nextId : channel.eventId);

	string ref = Util::valueOf(channel.serviceId) + "," + Util::valueOf(channel.transponderId);

	map<string, Channel*>::const_iterator it = m_channels.find(ref);

	if (it != m_channels.end()) {
		res.service = it->second->serv.ref;
	} else {
		res.service.type = RT_DVB;
		res.service.sid = channel.serviceId;
		res.service.tid = channel.transponderId & 0x0ffff;
		res.service.nid = (channel.transponderId >> 16) & 0x0ffff;
	}

	return res;
}

ProviderReader::ProviderReader() {

}

ProviderReader::~ProviderReader() {

}

void ProviderReader::handleLine(const vector<char*>& line) {
	if (line.size() < 2) {
		return;
	}

	int id = Util::getInt(line[0]);
	string name(line[1]);
	
	m_providers[id] = name;
}


void ProviderReader::finished() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "providers=" + Util::valueOf(m_providers.size()));
}

string ProviderReader::lookup(unsigned int id) const {
	string res;
	map<unsigned int, string>::const_iterator it = m_providers.find(id);

	if (it != m_providers.end()) {
		res = it->second;
	} else {
		// Log::getLogger()->log(Log::ERROR, "ADP", "provider not found: " + Util::valueOf(id));
	}

	return res;
}

TransponderReader::TransponderReader() {

}

TransponderReader::~TransponderReader() {

}

void TransponderReader::handleLine(const vector<char*>& line) {
	if (line.size() < 12) {
		return;
	}

	int pos = 0;
	Transponder t;
	unsigned int id = Util::getUInt(line[pos++]);
	
	t.nid = id >> 16 & 0x0ffff;
	t.tid = id & 0x0ffff;
	t.feType = Util::getInt(line[pos++]);
	t.freq = Util::getUInt(line[pos++]);
	t.pol = Util::getInt(line[pos++]);
	t.orbitPos = Util::getInt(line[pos++]);
	t.sr = Util::getUInt(line[pos++]);
	t.modulation = Util::getInt(line[pos++]);
	t.fec = Util::getInt(line[pos++]);
	t.pilot = Util::getInt(line[pos++]);
	t.rolloff = Util::getInt(line[pos++]);
	t.inversion = Util::getInt(line[pos++]);
	t.system = Util::getInt(line[pos]);
	
	m_transponders[id] = t;
}

void TransponderReader::finished() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "transponders=" + Util::valueOf(m_transponders.size()));
}

bool TransponderReader::lookup(unsigned int id, Transponder& t) const {
	bool res = false;
	map<unsigned int, Transponder>::const_iterator it = m_transponders.find(id);

	if (it != m_transponders.end()) {
		res = true;

		t = it->second;
	} else {
		// Log::getLogger()->log(Log::ERROR, "ADP", "transponder not found: " + Util::valueOf(id));
	}

	return res;
}

void TransponderReader::cleanup() {
	m_transponders.clear();
}


TitanChannelReader::TitanChannelReader() {

}

TitanChannelReader::~TitanChannelReader() {

}

void TitanChannelReader::handleLine(const vector<char*>& line) {
	if (line.size() < 12) {
		return;
	}

	int pos = 0;

	TitanChannel c;
	c.channelName = line[pos++];
	c.proc = Util::getInt(line[pos++]);
	c.serviceId = Util::getUInt(line[pos++]);
	c.transponderId = Util::getUInt(line[pos++]);
	c.channelList = line[pos++];
	c.serviceType = Util::getInt(line[pos++]);
	c.eventId = Util::getUInt(line[pos++]);
	c.actStart = Util::getTime(line[pos++]);
	c.actDesc = line[pos++];
	c.nextStart = Util::getTime(line[pos++]);
	c.nextStop = Util::getTime(line[pos++]);
	c.nextDesc = line[pos];

	if (line.size() > 12) {
		++pos;
		c.nextId = Util::getUInt(line[pos]);
	}

	m_channels.push_back(c);
}

void TitanChannelReader::finished() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "channels=" + Util::valueOf(m_channels.size()));
}

void TitanChannelReader::getEvents(ChannelReader& channels, EventList& events, bool nextEvent) const {
	std::list<TitanChannel>::const_iterator it = m_channels.begin();

	while (it != m_channels.end()) {
		events.push_back(channels.getEventFromChannel(*it, nextEvent));
		++it;
	}
}

TimerReader::TimerReader(ChannelReader& r): m_channelReader(r) {

}

TimerReader::~TimerReader() {

}

const TimerList& TimerReader::getTimers() const {
	return m_timers;
}

void TimerReader::handleLine(const vector<char*>& line) {
	if (line.size() < 12) {
		return;
	}

	Timer t;

	int pos = 0;
	t.begin = Util::getTime(line[pos++]);
	t.end = Util::getTime(line[pos++]);
	t.justPlay = Util::getInt(line[pos++]);
	t.repeat = Util::getInt(line[pos++]);
	t.name = line[pos++];

	string sid = line[pos++];
	string tid = line[pos++];
	string ref = sid + "," + tid;

	string name;
	Reference service;

	if (m_channelReader.lookup(ref, name, service)) {
		t.service = service;
	}

	t.state = (TimerState) Util::getTimerState(line[pos++]);
	++pos;
	t.id = line[pos++];
	t.serviceName = line[pos++];

	t.afterEvent = (AfterEvent) Util::getAfterEvent(line[pos]);

	m_timers.push_back(t);
}

void TimerReader::finished() {
	Log::getLogger()->log(Log::DEBUG, "ADP", "timers=" + Util::valueOf(m_timers.size()));
}


void TitanAdapter::about(ostream& os, const DeviceInfo& info, const Channel& channel) {
	os << "<e2abouts>";
	os << "<e2about>";


	os << "<e2enigmaversion>" << info.ver << "</e2enigmaversion>" << endl;
	os << "<e2imageversion>" << info.imageVer << "</e2imageversion>" << endl;
	os << "<e2webifversion>" << info.webIfVer << "</e2webifversion>" << endl;
	os << "<e2model>" << info.name << "</e2model>" << endl;
	
	os << "<e2tunerinfo>" << endl;

	std::list<Frontend>::const_iterator it = info.frontends.begin();
	
	while (it != info.frontends.end()) {
		const Frontend& f = *it;

		os << "<e2nim>";
		os << "<name>" << f.name << "</name>";
		os << "<type>" << f.model << "</type>";
		os << "</e2nim>" << endl;
		
		++it;
	}

	os << "</e2tunerinfo>" << endl;

	os << "<e2hddinfo>" << endl;

	if (info.hdds.empty()) {
		os << "<model>N/A</model>";
		os << "<capacity>-</capacity>";
		os << "<free>-</free>";
	}

	os << "</e2hddinfo>" << endl;

	os << "<e2servicename>" << channel.serv.name << "</e2servicename>" << endl;
	os << "<e2serviceprovider>" << channel.prov << "</e2serviceprovider>" << endl;
	os << "<e2servicevideosize>" << channel.width << "x" << channel.height << "</e2servicevideosize>" << endl;
	os << "<e2vpid>" << channel.vpid << "</e2vpid>" << endl;
	os << "<e2apid>" << channel.apid << "</e2apid>" << endl;
	os << "<e2tsid>" << channel.serv.ref.tid << "</e2tsid>" << endl;
	os << "<e2onid>" << channel.serv.ref.nid << "</e2onid>" << endl;
	os << "<e2sid>" << channel.serv.ref.sid << "</e2sid>" << endl;

	os << "</e2about>";
	os << "</e2abouts>";
}

bool TitanAdapter::setMute(bool on, Volume& vol) {
	/*string reply;

	string uri = "/queryraw?setmute&";
	
	if (on) {
		uri.append("1");
	} else {
		uri.append("0");
	}

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), uri, reply)) {
		vol.muted = (Util::getInt(reply) == 1);
	}

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getvol&100", reply)) {
		vol.val = Util::getInt(reply);
	}*/

	sendRc(113); // TODO: it seams that E2 uses 'flip-flop', so we send the RC and then get volume information
	getVolume(vol);

	return true; // TODO
}

bool TitanAdapter::setVolume(int val, Volume& vol) {
	string reply;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?setvol&" + Util::valueOf(val), reply)) {
		vol.val = Util::getInt(reply);
	}

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getmute", reply)) {
		vol.muted = (Util::getInt(reply) == 1);
	}

	return true; // TODO
}

bool TitanAdapter::getVolume(Volume& vol) {
	string reply;

	
	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getvol&100", reply)) {
		vol.val = Util::getInt(reply);
	}

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getmute", reply)) {
		vol.muted = (Util::getInt(reply) == 1);
	}

	return true; // TODO
}

EventList TitanAdapter::getEpgNextForBouquet(const string& name) {
	Log::getLogger()->log(Log::DEBUG, "ADP", "epgnext, bouquet='" + name + "'");
	string reply;
	EventList res;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getbouquetchannel&" + Util::urlEncode(m_serviceReader.getBouquetName(name), true), reply)) {
		TitanChannelReader r;
		istringstream is(reply);

		FileHelper::handleStream(is, r);

		r.getEvents(m_channelReader, res, true);
	}

	return res;
}

EventList TitanAdapter::getEpgNowForBouquet(const string& name) {
	Log::getLogger()->log(Log::DEBUG, "ADP", "epgnow, bouquet='" + name + "'");
	string reply;
	EventList res;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getbouquetchannel&" + Util::urlEncode(m_serviceReader.getBouquetName(name), true), reply)) {
		TitanChannelReader r;
		istringstream is(reply);

		FileHelper::handleStream(is, r);

		r.getEvents(m_channelReader, res);
	}

	return res;
}

TimerList TitanAdapter::getTimers() {
	TimerList res;
	string reply;

	if (Client::get(Config::getTitanHost(), Config::getTitanPort(), "/queryraw?getrectimer", reply)) {
		TimerReader r(m_channelReader);

		istringstream is(reply);

		FileHelper::handleStream(is, r);

		res = r.getTimers();
	}

	return res;
}

bool TitanAdapter::addTimer(const Timer& timer) {
	ostringstream os;

	os << "/queryraw?rectimersend";

	os << "&node=";
	
	if (!timer.id.empty()) {
		os << timer.id;
	} else {
		os << "0";
	}

	os << "&name=" << Util::urlEncode(timer.name, true);

	if (timer.justPlay) {
		os << "&type=switch";
	} else {
		os << "&type=record";
	}
	
	os << "&begin=" << Util::urlEncode(Util::getRecDate(timer.begin), true);
	os << "&end=" << Util::urlEncode(Util::getRecDate(timer.end), true);

	unsigned int tid = (timer.service.nid << 16) + timer.service.tid;

	os << "&sid=" << timer.service.sid << "&tid=" << tid;
	os << "&repeat=" << timer.repeat;
	os << "&afterevent=";

	if (timer.afterEvent == AE_AUTO) {
		os << "auto";
	} else if (timer.afterEvent == AE_DEEP_STANDBY) {
		os << "off";
	} else if (timer.afterEvent == AE_NONE) {
		os << "nothing";
	} else if (timer.afterEvent == AE_STANDBY) {
		os << "standby";
	}

	os << "&ext=1";

	string reply;

	return Client::get(Config::getTitanHost(), Config::getTitanPort(), os.str(), reply);
}

bool TitanAdapter::changeTimer(const Timer& timer) {
	return addTimer(timer);
}

bool TitanAdapter::deleteTimer(const Timer& timer) {
	bool deleted = false;

	TimerList timers = getTimers();
	
	for (TimerList::const_iterator it = timers.begin(); it != timers.end(); ++it) {
		const Timer&t = *it;

		if (t == timer) { // we found the timer to delete
			string reply;

			ostringstream os;
			os << "/queryraw?delrectimer&" << t.id;

			deleted = Client::get(Config::getTitanHost(), Config::getTitanPort(), os.str(), reply);
			break;
		}
	}

	return deleted;
}
