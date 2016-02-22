#ifndef ARA_LOG_BASE_H_201602
#define ARA_LOG_BASE_H_201602

#include "../ara_def.h"
#include "../dlist.h"
#include "../datetime.h"

#include <algorithm>
#include <iostream>
#include <functional>
#include <thread>
#include <mutex>

namespace ara {

	enum {
		g_nMaxCacheSize = 64 * 1024,
		g_nDefaultCacheSize = 32 * 1024,
	};

	class log_context;

	class log_stream : protected std::streambuf, public std::ostream
	{
	public:
		typedef char char_type;
		log_stream() : std::streambuf(), std::ostream((std::streambuf*)this)
			, cache_(nullptr), default_cache_size_(g_nDefaultCacheSize), max_cache_size_(g_nMaxCacheSize), cache_str_ptr_(nullptr) {
				cache_str_ptr_ = &local_cache_str_;
		}

		~log_stream() {
			delete_data();
		}

		void	set_cache_size(size_t n) {
			if (pbase() == nullptr)
				default_cache_size_ = n;
			else if (n > default_cache_size_) {
				char * tmp = new char[n + 1];
				tmp[n] = '\n';
				size_t rest = pptr() - pbase();
				memcpy(tmp, pbase(), rest);
				delete []cache_;
				cache_ = tmp;
				setp(cache_, cache_ + n);
				pbump(static_cast<int>(rest));
			}
		}
		void	set_max_cache_size(size_t n) {
			max_cache_size_ = n;
		}

	protected:
		void	release_context_cach();

		void	move_data(log_stream && s) {
		}
		void	delete_data() {
			if (cache_ == nullptr && pbase() != nullptr)
				release_context_cach();
			delete[]cache_;
			cache_ = nullptr;
			setp(nullptr, nullptr);
		}
		void	ref_to_context_cache() {
			log_context & context = get_log_context();
			char * p = context.get_log_cache();
			if (p != nullptr)
				setp(p, p + g_nDefaultCacheSize);
			cache_str_ptr_ = &context.get_cache_str();
		}

		int 	sync() {
			char * pBegin = pbase();
			char * pEnd = pptr();
			char * pBufEnd = epptr();

			if (pBegin == nullptr)
				return 0;
			else if (pBegin < pEnd) {
				if (pEnd == pBufEnd && pEnd > pBegin && *(pEnd - 1) != '\n')
					++pEnd;
					char * p = pBegin;
					while (pBegin < pEnd && (p = (char *)memchr(pBegin, '\n', pEnd - pBegin)) != NULL) {
						onCallBackData(pBegin, (++p) - pBegin);
						pBegin = p;
				}
				if (pBegin < pEnd)
					onCallBackData(pBegin, pEnd - pBegin);
			}
			setp(pBegin, pBufEnd);
			return 0;
		}
		int 	overflow(int c) {
			if (pbase() == nullptr) {
				cache_ = new char[default_cache_size_ + 1];
				cache_[default_cache_size_] = '\n';
				setp(cache_, cache_ + default_cache_size_);
			} else {
				size_t nSize = static_cast<size_t>(pptr() - pbase());
				if (nSize < max_cache_size_)
					set_cache_size(std::min<size_t>(max_cache_size_, static_cast<size_t>(nSize << 1)));
				else
					sync();
			}
			if (c != EOF) {
				* pptr() = (unsigned char)c;
				pbump(1);
			}
			return c;
		}
		inline std::string &	get_cache_str() { return *cache_str_ptr_; }
		virtual void onCallBackData(const char * s, size_t n) = 0;
	protected:
		char *					cache_ = nullptr;
		size_t					default_cache_size_;
		size_t					max_cache_size_;
		std::string				local_cache_str_;
		std::string *			cache_str_ptr_ = nullptr;
	private:
		log_stream(const log_stream & s) = delete;
		log_stream(log_stream && s) = delete;
		log_stream & operator=(const log_stream &) = delete;
		log_stream & operator=(log_stream &&) = delete;
	};

	class log
	{
	public:
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
	};
	
	namespace internal {
		class log_data
		{
		public:
			log_data(const date_time & t, const std::thread::id & tid, const ref_string & data, log::level l) :
				log_time_(t), thread_id_(tid), data_(data), level_(l) {}
			inline const date_time &		log_time() const { return log_time_; }
			inline const std::thread::id &	thread_id() const { return thread_id_; }
			inline const ref_string &		data() const { return data_; }
			inline log::level				get_level() const { return level_; }
		protected:
			const date_time		log_time_;
			std::thread::id		thread_id_;
			ref_string			data_;
			log::level			level_;
		};
	}

		class appender
		{
		public:
			appender() {}
			virtual ~appender() {}
			virtual bool		onWrite(const internal::log_data & data, const logger & l, std::string & cache_str) = 0;
			virtual std::string	dump_setting() const = 0;
			virtual void	flush_all() = 0;
		private:
			appender(const appender &) = delete;
			appender(appender &&) = delete;
			appender & operator = (const appender &) = delete;
			appender & operator = (appender &&) = delete;
		};
		typedef std::shared_ptr<appender>		appender_ptr;

		class logger
		{
		public:
			logger() : parent_(nullptr), level_(log::info), pass_to_parent_(false) {}
			logger(logger * parent, const std::string & name)
				: parent_(parent), name_(name), level_(log::debug), pass_to_parent_(true) {}

			appender_ptr 		get_appender() const {
				std::lock_guard<std::mutex> _guard(lock_);
				return appender_;
			}
			logger *				get_parent() const { return parent_; }
			level 					get_level() const { return level_; }
			const std::string &		get_name() const { return name_; }
			bool					get_pass_to_parent() const { return pass_to_parent_; }

			void	set_appender(const appender_ptr	& p) {
				std::lock_guard<std::mutex> _guard(lock_);
				appender_ = p;
			}
			void	set_pass_to_parent(bool b) { pass_to_parent_ = b; }
			void	set_level(level l) { level_ = l; }
			bool	can_display(level l) const {
				if (l > level_)
					return false;
				else if (appender_)
					return true;
				else if (pass_to_parent_ && parent_ != nullptr)
					return parent_->can_display(l);
				return false;
			}
			inline bool	can_display_in_this_logger(level l) const {
				return (l <= level_ && appender_);
			}
		protected:
			mutable std::mutex	lock_;
			logger *		parent_ = nullptr;
			appender_ptr	appender_;
			std::string		name_;
			log::level			level_ = info;
			bool			pass_to_parent_ = false;
		};

	namespace internal  {
		class log_context
		{
		public:
			log_context() {}
			~log_context() {}

			logger *  get_current_logger() { return logger_; }
			void           set_current_logger(logger &  l) { logger_ = &l; }

			std::string &  get_cache_str() { return cache_str_; }
		protected:
			logger *		logger_ = nullptr;
			std::string     cache_str_;
		};
	}
}

#endif//ARA_LOG_BASE_H_201602

