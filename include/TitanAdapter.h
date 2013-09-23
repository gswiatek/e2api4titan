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

#ifndef _gs_e2_TitanAdapter_h_
#define _gs_e2_TitanAdapter_h_

#include "LineHandler.h"
#include "WebApi.h"

#include <map>
#include <cstdlib>

namespace gs {
	namespace e2 {
		// taken from http://radiovibrations.com/dreambox/namespace.htm
		inline bool isValid(unsigned int nid, unsigned int tid, int pos) {
			if (nid == 0 || nid == 0x1111) {
				return false;
			} else if (nid == 1) {
				return (pos == 192);
			} else if (nid == 0x00b1) {
				return tid != 0x00b0;
			} else if (nid == 2) {
				return abs(pos - 282) < 6;
			} else {
				return nid < 0xff00;
			}
		}

		// taken from http://radiovibrations.com/dreambox/namespace.htm
		inline int getDvbNamespace(unsigned int nid, unsigned int tid, int pos, unsigned int freq, int pol) {
			int res = pos << 16;

			if (!isValid(nid, tid, pos)) {
				res |= ((freq / 1000) & 0x0ffff) | ((pol & 1) << 15);
			}

			return res;
		}


		typedef struct transponder {
			unsigned int nid;
			unsigned int tid;
			int feType;
			unsigned int freq;
			int pol;
			int orbitPos;
			unsigned int sr;
			int modulation;
			int fec;
			int pilot;
			int rolloff;
			int inversion;
			int system;
		} Transponder;

		typedef struct titanChannel {
		public:
			titanChannel(): proc(0), serviceId(0), transponderId(0), serviceType(0), eventId(0), actStart(0), nextStart(0), nextStop(0), nextId(0) {

			}
			std::string channelName;
			int proc;
			unsigned int serviceId;
			unsigned long long transponderId;
			std::string channelList;
			int serviceType;
			unsigned int eventId;
			time_t actStart;
			std::string actDesc;
			time_t nextStart;
			time_t nextStop;
			std::string nextDesc;
			unsigned int nextId;
		} TitanChannel;

		class TransponderReader: public LineHandler {
		public:
			TransponderReader();
			virtual ~TransponderReader();

			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();
			bool lookup(unsigned int id, Transponder& transponder) const;
			void cleanup();

		private:
			std::map<unsigned int, Transponder> m_transponders;
		};

		class EpgReader: public LineHandler {
		public:
			EpgReader();
			virtual ~EpgReader();

			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();

			const EventList& getEvents() const;

		private:
			EventList m_events;
		};

		class ProviderReader: public LineHandler {
		public:
			ProviderReader();
			virtual ~ProviderReader();
			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();
			std::string lookup(unsigned int id) const;
		private:
			std::map<unsigned int, std::string> m_providers;
		};

		class ChannelReader: public LineHandler {
		public:
			ChannelReader(ProviderReader& providerReader, TransponderReader& transponderReader);
			virtual ~ChannelReader();
			void handleLine(const std::vector<std::string>& line);
			virtual void finished();
			bool lookup(const std::string& id, Channel& channel);
			bool lookup(const std::string& id, std::string& name, Reference& reference) const;
			Event getEventFromChannel(const TitanChannel& channel, bool nextEvent = false) const;
			void cleanup();
		private:
			ProviderReader& m_providerReader;
			TransponderReader& m_transponderReader;
			std::map<std::string, Channel*> m_channels; // this list will be destroyed after reading of bouquets
			std::map<std::string, Service> m_usedChannels; // channels referenced in bouquets will be cached for the lifetime
		};

		class ServiceReader: public LineHandler {
		public:
			ServiceReader(ChannelReader& reader, bool bouquetFile = false, bool radioBouquet = false);
			virtual ~ServiceReader();
			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();
			const ServiceList& getServices() const;
			const ServiceList& getRadioServices() const;
			const ServiceList& getTvServices() const;
			const ServiceList& getServices(const std::string& bouquetName) const;
			const std::string& getBouquetName(const std::string& ref) const;
			const std::string& getBouquetFile(const std::string& bouquteName) const;
			void setBouquetPath(const std::string& bouquetPath);

		private:
			std::string m_bouquetPath;
			ChannelReader& m_channelReader; 
			bool m_bouquetFile;
			bool m_radioBouquet;
			ServiceList m_services; // all services
			ServiceList m_radioServices; // only radio
			ServiceList m_tvServices; // only tv
			ServiceList m_empty;
			std::map<std::string, ServiceReader*> m_readers; //  lookup table with bouquet file name as key
			std::map<std::string, std::string> m_bouquetName; // lookup table with bouquet file name as key and bouquet name as value
			std::map<std::string, std::string> m_bouquetFileName; // lookup table with bouquet name as key and bouquet file name as value
		};

		class MovieReader: public LineHandler {
		public:
			MovieReader();
			virtual ~MovieReader();

			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();

			const std::list<std::string>& getMovies() const;

		private:
			std::list<std::string> m_movies;
		};
		
		// Responsible for reading row channels information from Titan using Web API
		class TitanChannelReader: public LineHandler {
		public:
			TitanChannelReader();
			virtual ~TitanChannelReader();

			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();

			/** Gets E2 events from the channel information */
			void getEvents(ChannelReader& channels, EventList& l, bool next = false) const;

		private:
			std::list<TitanChannel> m_channels;
		};

		/** Used to read Titan timers */
		class TimerReader: public LineHandler {
		public:
			TimerReader(ChannelReader& channelReader);
			virtual ~TimerReader();

			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();

			const TimerList& getTimers() const;

		private:
			ChannelReader& m_channelReader; 
			TimerList m_timers;
		};

		/** Used to read events from EPG search */
		class EpgSearchReader: public LineHandler {
		public:
			EpgSearchReader(ChannelReader& reader);
			virtual ~EpgSearchReader();

			virtual void handleLine(const std::vector<std::string>& line);
			virtual void finished();

			const EventList& getEvents() const;

		private:
			ChannelReader& m_channelReader;
			Service m_lastService;
			EventList m_events;
		};

		class TitanAdapter {
		public:
			TitanAdapter();
			~TitanAdapter();

			static TitanAdapter* getAdapter();

			static void about(std::ostream& os, const DeviceInfo& info, const Channel& channel);

			void init();
			bool isRunning();
			const ServiceList& getServices() const;
			const ServiceList& getServices(const std::string& reference) const;
			void about(DeviceInfo& device, Channel& currentService);
			void deviceInfo(DeviceInfo& device);
			std::list<std::string> getLocations();
			bool zap(const std::string& channelReference);
			EventList getEpg(const std::string& channelReference);
			EventList getEpgNow(const std::string& channelReference);
			EventList getEpgNowForBouquet(const std::string& bouquetName);
			EventList getEpgNextForBouquet(const std::string& bouquetName);
			EventList getEpgNext(const std::string& channelReference);
			EventList searchEpg(const std::string& searchQuery);
			bool setPowerState(PowerState state);
			CurrentService getCurrent(bool withEpg = true);
			MovieList getMovies();
			std::string getRcName(int code);
			bool sendRc(int code);
			bool deleteMovie(const std::string& ref);
			bool getVolume(Volume& vol);
			bool setMute(bool on, Volume& vol);
			bool setVolume(int val, Volume& vol);
			TimerList getTimers();
			bool addTimer(const Timer& t);
			bool changeTimer(const Timer& t);
			bool deleteTimer(const Timer& t);
			bool getEpg(const std::string& ref, unsigned int id, Event& event);
			Reference getCurrentReference();
			bool sendMessage(const std::string& title, const std::string& msg, int timeout);

		private:
			bool getEpg(const std::string& ref, Event& event);
			bool getActive(TitanChannel& currentService);

			/** Read information about the receiver and return the configuration information from titan.cfg */
			std::map<std::string, std::string> readInfo();
			void readBouquets(const std::string& fileName, const std::string& pathName);
			void readChannels(const std::string& fileName);
			void readProviders(const std::string& fileName);
			void readTransponders(const std::string& fileName);

			DeviceInfo m_info;
			TransponderReader m_transponderReader;
			ProviderReader m_providerReader;
			ChannelReader m_channelReader;
			ServiceReader m_serviceReader;
			std::map<int, std::string> m_rcCodes;

			static TitanAdapter* m_adapter; // we store here the instance
		};

		
	}
}

#endif
