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

#ifndef _gs_e2_FileHelper_h_
#define _gs_e2_FileHelper_h_

#include <string>
#include <iostream>

namespace gs {
	namespace e2 {
		class LineHandler;

		class FileHelper {
		public:
			/** We perform here parsing of the Titan specific files. Each parsed line will be delegated to the handler */
			static void readConfigFile(const std::string& fileName, LineHandler& handler);

			static void handleStream(std::istream& is, LineHandler& handler);

			static inline int readLine(std::istream& is, char* buf, int max) {
				int count = 0;		
				int m = max - 1;

				int c = is.get();
 		
				while (c != -1 && c != '\n' && count < m)  {
					buf[count++] = c;
					c = is.get();
				}

				buf[count] = 0;

				if (count > 0 && buf[count - 1] == '\r') {
					--count;
					buf[count] = 0;
				}

				if (c == -1 && count == 0)
					count = -1;

				return count;
			}
		private:
			FileHelper();
			~FileHelper();
		};
	}
}

#endif
