#ifndef ARA_LOG_BASE_H_201602
#define ARA_LOG_BASE_H_201602

#include "../ara_def.h"
#include "../dlist.h"
#include "../datetime.h"
#include "strformat.h"

#include <algorithm>
#include <iostream>
#include <functional>
#include <thread>
#include <mutex>

namespace ara {

	enum {
		g_nMaxCacheSize = 64 * 1024 * 1024,
		g_nDefaultCacheSize = 32 * 1024,
	};

	namespace log {
		enum level
		{
			emergency = 0,
			alert,				//! A condition that should be corrected  immediately, such as a corrupted system database.
			critical,			//! Critical conditions, such as hard device errors.
			error,				//! Errors.
			warning,			//! Warning messages.
			notice,				//! Conditions that are not error conditions, but that may require special handling.
			info,				//! Informational messages.
			debug				//! Messages that contain information normally of use only when debugging a program.
		};

		class logger;

		class log_context
		{
		public:
			log_context() noexcept {}
			~log_context() {
				release_cache();
			}

			log::logger *  get_current_logger() { return logger_; }
			void      set_current_logger(log::logger *  l) { logger_ = l; }

			char *		get_log_cache(size_t n) {
				if (cache_ && cache_size_ < n) {
					release_cache();
				}
				if (!cache_) {
					cache_ = new char[n];
					cache_size_ = n;
				}
				return cache_;
			}
			void release_cache() {
				if (cache_) {
					delete[]cache_;
					cache_ = nullptr;
					cache_size_ = 0;
				}
			}
		protected:
			log::logger *	logger_ = nullptr;
			std::string     cache_str_;
			char *			cache_ = nullptr;
			size_t			cache_size_ = 0;
		};
	}
}

#endif//ARA_LOG_BASE_H_201602

