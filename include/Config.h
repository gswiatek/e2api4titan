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

#ifndef _gs_e2_Config_h_
#define _gs_e2_Config_h_

#include <string>

namespace gs {
	namespace e2 {

		class Config {
		public:
			static bool parse(int argc, const char** argv);
			static void usage(const char* progName);

			static const bool& isDaemon();
			static const int& getPort();
			static const int& getThreads();
			static const int& getDataPort();
			static const std::string& getTitanHost();
			static const int& getTitanPort();
			static const int& getTitanDataPort();
			static const std::string& getTitanDir();
			static const std::string& getEtcDir();
			static const std::string& getLogFile();
			static std::size_t getMaxLogFileSize();

		private:
			Config();
			~Config();

			static bool m_daemon; /**< Should we start as daemon (default: false) */
			static int m_port; /**< Port for the web server (default: 8080) */
			static int m_threads; /**< Number of threads used in pool (default: 2) */
			static std::string m_titanHost; /**< IP for Titan receiver (default: 127.0.0.1) */
			static int m_titanPort; /**< Web server port on Titan receiver (default: 80) */
			static int m_titanDataPort; /**< Data port on Titan receiver (default: 22222) */
			static int m_dataPort; /**< Enigma2 data port (default: 8001) */
			static std::string m_titanDir; /**< Directory where the bouquets and channels are stored (/var/etc/titan) */
			static std::string m_etcDir; /**< etc directory with firmware information (/etc) */
			static std::string m_logFile; /**< The file used for logging (default: /tmp/e2webserv.log */
			static std::size_t m_maxLogFileSize; /** Max log file size in KB (default: 10) */
		};
	}
}


#endif
