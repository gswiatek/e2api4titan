#ifndef _gs_e2_Mutex_h_
#define _gs_e2_Mutex_h_

#ifdef _WIN32

#else
#include <pthread.h>
#endif

namespace gs {
	namespace e2 {
		/** Mutex used for synchronization of concurrent access to some resource */
		class Mutex {
		public:
			Mutex();
			~Mutex();

			/** acquire the lock */
			void lock();
			/** release the lock */
			void unlock();
		private:
			Mutex(const Mutex&); // disable copy constructor
			Mutex& operator=(const Mutex&); // disable assign operator

#ifdef _WIN32

#else
			pthread_mutex_t m_lock;		
#endif
		};

		class Guard {
		public:
			Guard(Mutex& m);
			~Guard();

		private:
			Mutex& m_mutex;
		};
	}
}
#endif
