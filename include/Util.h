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

#ifndef _gs_e2_Util_h_
#define _gs_e2_Util_h_

#include <string>
#include <vector>
#include <ctime>
#include <iostream>

namespace gs {
	namespace e2 {
		class Util {
		public:
			/** Performs XML escaping of the given input string */
			static std::string getXml(const std::string& str);
			/** Constructs Titan reference (sid,tid) from E2 reference */
			static std::string getTitanRef(const std::string& e2Ref);
			/** Extracts unsigned int value from a string */
			static unsigned int getUInt(const std::string& data);
			/** Extracts int value from a string */
			static int getInt(const std::string& data);
			/** Extracts time_t value from a string */
			static time_t getTime(const std::string& data);
			/** Extracts time from Titan specific file name format (used to get recording timestamp for a movie) */
			static time_t parseTime(const std::string& fileName);
			/** Computes current date/time string to be used in HTTP response */
			static std::string getHttpDate();
			/** Parses Titan specific lines with values separated by # and delievers the extracted tokens */
			static std::vector<std::string> getTokens(const std::string& line);
			/** Converts int value to string */
			static std::string valueOf(int val);
			/** Converts unsigned int value to string */
			static std::string valueOf(unsigned int val);

			static int urlDecode(const char* src, int src_len, char* dst, int dst_len);
			/**
			 * URL encoding
			 *
			 * @param data input data to be encoded
			 * @param mongooseClient the client download uses _vsnprintf so we must double escape the perecent char (%)
			 */
			static std::string urlEncode(const std::string& data, bool mongooseClient = false);
			/** Removes all trailing whitespace of the input string */
			static void rtrim(std::string& str);
			/** Removes all leading whitespaces of the input string */
			static void ltrim(std::string& str);
			/** Removes all trailing and leading whitespace of the input string */
			static void trim(std::string& str);
		private:
			Util();
			~Util();
		};
	}
}

#endif
