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

#ifndef _gs_e2_WebApi_h_
#define _gs_e2_WebApi_h_

#include <string>
#include <list>
#include <iostream>
#include <ios>
#include <ctime>

#include "Util.h"

namespace gs {
	namespace e2 {
		typedef struct frontend {
			std::string name;
			std::string model;
		} Frontend;

		typedef struct hdd {
			std::string name;
			std::string capacity;
			std::string free;
		} Hdd;

		typedef struct network_if {
			std::string name;
			std::string mac;
			std::string ip;
			std::string gateway;
			std::string mask;
			bool dhcp;
		} NetworkIf;

		typedef struct device_info {
			std::string name;
			std::string ver;
			std::string imageVer;
			std::string webIfVer;
			std::list<Frontend> frontends;
			std::list<NetworkIf> interfaces;
			std::list<Hdd> hdds;
		} DeviceInfo;


		typedef enum {
			RT_INVALID = -1,
			RT_STRUCTURE,
			RT_DVB,
			RT_FILE,
			RT_USER = 0x1000
		} ReferenceType;

		typedef enum {
			FT_DIRECTORY = 1,
			FT_MUST_DESCENT = 2,
			FT_CAN_DESCENT = 4,
			FT_SHOULD_SORT = 8,
			FT_HAS_SORT_KEY = 16,
			FT_SORT1 = 32,
			FT_IS_MARKER = 64,
			FT_IS_GROUP = 128,
			FT_IS_MULTICAST = 0x10000,
			FT_MUST_JOIN_MULTICAST_GROUP = 0x20000
		} FlagType;

		typedef struct reference {
		public:
			reference(): type(RT_DVB), flags(0), tv_radio(1), sid(0), tid(0), nid(0), dvbName(0), unknown1(0), unknown2(0), unknown3(0)  {

			}

			ReferenceType type;
			unsigned int flags; // bit-mask
			int tv_radio; // 1 TV, 2 radio
			unsigned int sid;  // service id
			unsigned int tid; // transponder id
			unsigned int nid; // network id
			int dvbName; // DVB namespace: stores information about orbital position and eventually the frequency and polartisation
			int unknown1;
			int unknown2;
			int unknown3;
			std::string path;
		} Reference;

		typedef struct service {
			bool bouquet;
			Reference ref;
			std::string name;
		} Service;

		typedef std::list<Service> ServiceList;

		typedef struct channel {
		public:
			channel(): vpid(0), apid(0), width(0), height(0) {
	
			}

			Service serv;
			std::string prov;
			unsigned int vpid; // video pid
			unsigned int apid; // audio pid
			unsigned int width; // video width
			unsigned int height; // video height
		} Channel;

		typedef struct movie {
		public:
			movie(): recTime(0), len(0), fileSize(0) {
				ref.tv_radio = 0;
			}

			Reference ref;
			std::string title;
			std::string desc;
			std::string extended;
			std::string serviceName;
			time_t recTime;
			int len;
			std::string fileName;
			size_t fileSize;
		} Movie;

		typedef std::list<Movie> MovieList;

		typedef struct event {
		public:
			event(): id(0), start(0), current(0), dur(0) {}
			unsigned int id;
			time_t start;
			time_t current;
			time_t dur;
			std::string title;
			std::string desc;
			std::string extended;
		} Event;

		typedef std::list<Event> EventList;

		typedef struct current {
			Service service;
			EventList events;
		} CurrentService;

		typedef struct volume {
		public:
			volume(): val(100), muted(false) {}
			int val;
			bool muted;
		} Volume;

		typedef enum  {PS_TOOGLE_STANDBY, PS_DEEP_STANDBY, PS_REBOOT, PS_RESTART, PS_WAKEUP, PS_STANDBY} PowerState;

		inline std::ostream& operator<<(std::ostream& os, const Reference& ref) {
			os << std::hex << ref.type << ':'  << ref.flags << ':' <<  ref.tv_radio << ':'
				<< ref.sid << ':' << ref.tid << ':' <<  ref.nid << ':'  << ref.dvbName << ':' 
				<< ref.unknown1 << ':' << std::hex << ref.unknown2 << ':' << ref.unknown3 << ':' << std::dec;
			
			if (!ref.path.empty()) {
				os << Util::getXml(ref.path);
			}

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const Service& service) {

			os << "<e2service>" << std::endl;
			os << "<e2servicereference>" << service.ref << "</e2servicereference>" << std::endl;
			os << "<e2servicename>" << Util::getXml(service.name) << "</e2servicename>" << std::endl;
			os << "</e2service>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const ServiceList& services) {

			os << "<e2servicelist>" << std::endl;

			typedef std::list<Service>::const_iterator ServiceConstIterator;

			ServiceConstIterator it = services.begin();

			while (it != services.end()) {
				os << *it;
				++it;
			}

			os << "</e2servicelist>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const Frontend& frontend) {
			os << "<e2frontend>" << std::endl;
			os << "<e2name>" << Util::getXml(frontend.name) << "</e2name>" << std::endl;
			os << "<e2model>" << Util::getXml(frontend.model) << "</e2model>" << std::endl;
			os << "</e2frontend>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const DeviceInfo& info) {

			os << "<e2deviceinfo>" << std::endl;

			os << "<e2enigmaversion>" << Util::getXml(info.ver) << "</e2enigmaversion>" << std::endl;
			os << "<e2imageversion>" << Util::getXml(info.imageVer) << "</e2imageversion>" << std::endl;
			os << "<e2webifversion>" << info.webIfVer << "</e2webifversion>" << std::endl;
			os << "<e2devicename>" << Util::getXml(info.name) << "</e2devicename>" << std::endl;
			os << "<e2frontends>" << std::endl;

			typedef std::list<Frontend>::const_iterator FrontendConstIterator;
			FrontendConstIterator it = info.frontends.begin();

			while (it != info.frontends.end()) {
				os << *it;
				++it;
			}

			os << "</e2frontends>" << std::endl;

			os << "</e2deviceinfo>" << std::endl;
			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const Event& event) {

			os << "<e2event>" << std::endl;
			os << "<e2eventid>" << event.id << "</e2eventid>" << std::endl;
			os << "<e2eventstart>" << event.start << "</e2eventstart>" << std::endl;
			os << "<e2eventduration>" << event.dur << "</e2eventduration>" << std::endl;
			os << "<e2eventcurrenttime>" << event.current << "</e2eventcurrenttime>" << std::endl;
			os << "<e2eventtitle>" << Util::getXml(event.title) << "</e2eventtitle>" << std::endl;
			os << "<e2eventdescription>" << Util::getXml(event.desc) << "</e2eventdescription>" << std::endl;
			os << "<e2eventdescriptionextended>" << Util::getXml(event.extended) << "</e2eventdescriptionextended>" << std::endl;
			os << "</e2event>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const Channel& channel) {

			if (channel.apid > 0) {
				os << "<e2apid>" << channel.apid << "</e2apid>" << std::endl;
			}

			if (channel.vpid > 0) {
				os << "<e2vpid>" << channel.vpid << "</e2vpid>" << std::endl;
			}

			os << "<e2tsid>" << channel.serv.ref.tid << "</e2tsid>" << std::endl;
			os << "<e2onid>" << channel.serv.ref.nid << "</e2onid>" << std::endl;
			os << "<e2sid>" << channel.serv.ref.sid << "</e2sid>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, EventList& events) {

			os << "<e2eventlist>" << std::endl;

			typedef std::list<Event>::iterator EventIterator;

			EventIterator it = events.begin();
			time_t t = time(0);

			while (it != events.end()) {
				(*it).current = t;
				os << *it;
				++it;
			}

			os << "</e2eventlist>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const Movie& movie) {

			os << "<e2movie>" << std::endl;
			os << "<e2servicereference>" << movie.ref << "</e2servicereference>" << std::endl;
			os << "<e2title>" << Util::getXml(movie.title) << "</e2title>" << std::endl;
			os << "<e2description>" << Util::getXml(movie.desc) << "</e2description>" << std::endl;
			os << "<e2descriptionextended>" << Util::getXml(movie.extended) << "</e2descriptionextended>" << std::endl;
			os << "<e2servicename>" << Util::getXml(movie.serviceName) << "</e2servicename>" << std::endl;
			if (movie.recTime > 0) {
				os << "<e2time>" << movie.recTime << "</e2time>" << std::endl;
			}

			if (movie.len > 0) {
				os << "<e2length>" << movie.len << "</e2length>" << std::endl;
			}
			os << "<e2filename>" << Util::getXml(movie.fileName) << "</e2filename>" << std::endl;

			if (movie.fileSize > 0) {
				os << "<e2filesize>" << movie.fileSize << "</e2filesize>" << std::endl;
			}

			os << "</e2movie>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, const MovieList& movies) {

			os << "<e2movielist>" << std::endl;

			typedef std::list<Movie>::const_iterator MovieConstIterator;

			MovieConstIterator it = movies.begin();

			while (it != movies.end()) {
				os << *it;
				++it;
			}

			os << "</e2movielist>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, CurrentService& currentService) {

			os << "<e2currentserviceinformation>" << std::endl;

			os << currentService.service;
			os << currentService.events;

			os << "</e2currentserviceinformation>" << std::endl;

			return os;
		}

		inline std::ostream& operator<<(std::ostream& os, Volume& volume) {

			os << "<e2volume>" << std::endl;

			bool res = true; // TODO
			os << "<e2result>" << res << "</e2result>" << std::endl;
			os << "<e2current>" << volume.val << "</e2current>" << std::endl;
			os << "<e2ismuted>" << (volume.muted ? "true": "false") << "</e2ismuted>" << std::endl;

			os << "</e2volume>" << std::endl;

			return os;
		}
	}
}

#endif
