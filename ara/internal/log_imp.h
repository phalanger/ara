#ifndef ARA_LOG_IMP_H_201603
#define ARA_LOG_IMP_H_201603

#include "../ara_def.h"
#include "../dlist.h"
#include "../datetime.h"
#include "../threadext.h"
#include "../ref_string.h"
#include "../singleton.h"
#include "strformat.h"

#include <algorithm>
#include <iostream>
#include <functional>
#include <thread>
#include <mutex>

namespace ara {

	namespace log {
		class log_stream : protected std::streambuf, public std::ostream
		{
		public:
			typedef char char_type;
			log_stream() : std::streambuf(), std::ostream((std::streambuf*)this)
				, cache_(nullptr), default_cache_size_(g_nDefaultCacheSize), max_cache_size_(g_nMaxCacheSize) {
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
					delete[]cache_;
					cache_ = tmp;
					setp(cache_, cache_ + n);
					pbump(static_cast<int>(rest));
				}
			}
			void	set_max_cache_size(size_t n) {
				max_cache_size_ = n;
			}

		protected:
			void	delete_data() {
				if (cache_ != nullptr) {
					delete[]cache_;
					cache_ = nullptr;
				}
				setp(nullptr, nullptr);
			}
			void	ref_to_context_cache() {
				log_context & context = thread_context::get()._get_log_context();
				char * p = context.get_log_cache(default_cache_size_);
				if (p != nullptr)
					setp(p, p + default_cache_size_);
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
					ref_to_context_cache();
				}
				else {
					size_t nSize = static_cast<size_t>(pptr() - pbase());
					if (nSize < max_cache_size_)
						set_cache_size(std::min<size_t>(max_cache_size_, static_cast<size_t>(nSize << 1)));
					else
						sync();
				}
				if (c != EOF) {
					*pptr() = (unsigned char)c;
					pbump(1);
				}
				return c;
			}
			virtual void onCallBackData(const char * s, size_t n) = 0;
		protected:
			char *					cache_ = nullptr;
			size_t					default_cache_size_;
			size_t					max_cache_size_;
		private:
			log_stream(const log_stream & s) = delete;
			log_stream(log_stream && s) = delete;
			log_stream & operator=(const log_stream &) = delete;
			log_stream & operator=(log_stream &&) = delete;
		};

		class log_data
		{
		public:
			log_data(const date_time & t, const std::thread::id & tid, const ref_string & data, log::level l) :
				log_time_(t), thread_id_(tid), data_(data), level_(l) {}
			inline const date_time &		log_time() const { return log_time_; }
			inline const std::thread::id &	thread_id() const { return thread_id_; }
			inline const ref_string &		data() const { return data_; }
			inline log::level				get_level() const { return level_; }

			inline static const char *		get_level_name(level l) {
				static const char * p[] =	{
					"Emergency",
					"Alert",
					"Critical",
					"Error",
					"Warning",
					"Notice",
					"Info",
					"Debug"
				};
				return p[static_cast<int>(l)];
			}
		protected:
			const date_time		log_time_;
			std::thread::id		thread_id_;
			ref_string			data_;
			log::level			level_;
		};

		class appender
		{
		public:
			appender() {}
			virtual ~appender() {}
			virtual bool		onWrite(const log_data & data, const logger & l, std::string & cache_str) = 0;
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
			log::level 				get_level() const { return level_; }
			const std::string &		get_name() const { return name_; }
			bool					get_pass_to_parent() const { return pass_to_parent_; }

			void	set_appender(const appender_ptr	& p) {
				std::lock_guard<std::mutex> _guard(lock_);
				appender_ = p;
			}
			void	set_pass_to_parent(bool b) { pass_to_parent_ = b; }
			void	set_level(log::level l) { level_ = l; }
			bool	can_display(log::level l) const {
				if (l > level_)
					return false;
				else if (appender_)
					return true;
				else if (pass_to_parent_ && parent_ != nullptr)
					return parent_->can_display(l);
				return false;
			}
			inline bool	can_display_in_this_level(log::level l) const {
				return (l <= level_);
			}
		protected:
			mutable std::mutex	lock_;
			logger *		parent_ = nullptr;
			appender_ptr	appender_;
			std::string		name_;
			log::level		level_ = log::info;
			bool			pass_to_parent_ = false;
		};

		class logger_mgr
		{
		public:
			static logger_mgr	& get() {
				return singleton<logger_mgr>::get();
			}

			logger_mgr() : root_(nullptr, "Root") {
			}
			~logger_mgr() {
				for (auto it : mapLogger_)
					delete it.second;
			}
			
			logger *	get_logger(const std::string & name) {

				std::lock_guard<std::mutex>		_aguard(lock_);

				auto itFind = mapLogger_.find(name);
				if (itFind != mapLogger_.end())
					return itFind->second;

				std::string::size_type p = name.rfind('.');
				if (p == std::string::npos) {
					auto ptr = new logger(&root_, name);
					mapLogger_[name] = ptr;
					return ptr;
				} 
				auto ptr = new logger(get_logger(name.substr(0, p)), name);
				mapLogger_[name] = ptr;
				return ptr;
			}

			logger * get_root() {
				return &root_;
			}

		protected:
			std::mutex		lock_;
			std::map<std::string, logger *>		mapLogger_;
			logger			root_;
		};

		class log_imp : public log_stream
		{
		public:
			typedef str_format<log_stream>	typeFormat;

			log_imp(logger * logger) : formator_(*((log_stream *)this)),logger_(logger) {}

			log_imp &	operator()(log::level l) {
				level_ = l;
				return *this;
			}

			template<class ch, typename...TypeList>
			inline log_stream &	printf(const ch * s, const TypeList&... t2) {
				formator_.printf(s, t2...);
				return *this;
			}

			template<class ch, typename...TypeList>
			inline log_stream &	printfln(const ch * s, const TypeList&... t2) {
				formator_.printf(s, t2...);
				*this << std::endl;
				return *this;
			}

		protected:
			inline void	set_logger(logger * l) {
				logger_ = l;
			}
			inline logger * get_logger() const {
				return logger_;
			}

			virtual void onCallBackData(const char * s, size_t n) {
				if (!logger_)
					return;
				log_data	data(date_time::get_current(), std::this_thread::get_id(), ref_string(s, n), level_);
				logger	* p = logger_;
				while (p != nullptr && p->can_display_in_this_level(level_)) {
					if (p->get_appender())
						p->get_appender()->onWrite(data, *p, cache_str_);
					if (!p->get_pass_to_parent())
						break;
					p = p->get_parent();
				}
			}
			typeFormat		formator_;
			log::level		level_ = log::info;
			logger	*		logger_ = nullptr;
			std::string		cache_str_;
		};
	};//log
}

#endif//ARA_LOG_IMP_H_201603

