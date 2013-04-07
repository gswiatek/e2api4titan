#ifndef _gs_e2_Log_h_
#define _gs_e2_Log_h_

#include "Mutex.h"

#include <string>
#include <fstream>

namespace gs {
	namespace e2 {
		/** Simple log implementation with one backup of last file after file size limit */
		class Log {
		public:
			typedef enum {
				ERROR,
				INFO,
				DEBUG
			} Level;

			Log(const std::string& fileName, std::size_t maxSize);
			~Log();

			void log(Level l, const std::string& category, const std::string& msg);

			static Log* getLogger();
			static void setLogger(Log* log);

		private:
			static const char* m_levelNames[DEBUG + 1];
			static Log* m_log;

			void backup(); // backups the current file and opens the logger again.

			std::string m_fileName;
			std::size_t m_maxSize;
			std::size_t m_pos;
			Mutex m_mutex;
			std::fstream m_os;
		};
	}
}

#endif
