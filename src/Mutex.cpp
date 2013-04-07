#include "Mutex.h"

using namespace gs::e2;

Mutex::Mutex() {
#ifdef _WIN32

#else
	pthread_mutex_init(&m_lock, 0);
#endif
}

Mutex::~Mutex() {
#ifdef _WIN32

#else
	pthread_mutex_destroy(&m_lock);
#endif
}

void Mutex::lock() {
#ifdef _WIN32

#else
	pthread_mutex_lock(&m_lock);
#endif
}

void Mutex::unlock() {
#ifdef _WIN32

#else
	pthread_mutex_unlock(&m_lock);
#endif
}

Guard::Guard(Mutex& m): m_mutex(m) {
	m_mutex.lock();
}

Guard::~Guard() {
	m_mutex.unlock();
}
