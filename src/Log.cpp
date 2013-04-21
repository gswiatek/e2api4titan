#include "Log.h"

#include <sstream>
#include <ctime>

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace gs::e2;
using namespace std;

const char* Log::m_levelNames[DEBUG + 1] = {"ERR ", "INF ", "DBG "};

Log* Log::m_log = 0;

Log* Log::getLogger() {
	return m_log;
}

void Log::setLogger(Log* log) {
	m_log = log;
}

Log::Log(const std::string& fileName, size_t maxSize): m_fileName(fileName), m_maxSize(maxSize), m_pos(0) {
	m_os.open(m_fileName.c_str(),  ios::out | ios::app);

	if (m_os.is_open()) {
		m_pos = m_os.tellp();
	}
}

Log::~Log() {
	m_os.close();
}

void Log::backup() {
	m_os.close();

	// TODO: windows
#ifndef _WIN32
	string backupFile = m_fileName + ".1";
	unlink(backupFile.c_str()); // delete old backup
	rename(m_fileName.c_str(), backupFile.c_str());
#endif

	m_os.open(m_fileName.c_str(), ios::out | ios::app);
	m_pos = 0;
}

void Log::log(Level l, const string& category, const string& msg) {

	ostringstream os;
	os << time(0) << " [";

#ifndef _WIN32
	os << std::hex << pthread_self();
#endif
	os << "] " << m_levelNames[l] << category <<  " -- " << msg;

	string toLog = os.str();

	Guard g(m_mutex);

	if ((m_pos + toLog.size()) > m_maxSize) { // we need to create backup file
		backup();
	}

	if (m_os.is_open()) {
		m_os << toLog << endl;
		m_pos = m_os.tellp();
	}
}