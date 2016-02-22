#ifndef ARA_LOG_CONTEXT_H_201602
#define ARA_LOG_CONTEXT_H_201602

#include "../ara_def.h"
#include "../dlist.h"
#include "../datetime.h"

#include <vector>
#include <atomic>
#include <mutex>
#include <iostream>
#include <functional>

namespace ara {
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

		namespace internal {
			class log_data
			{
			public:
				log_data(const date_time & t, const std::thread::id & tid, const ref_string & data, level l) :
					log_time_(t), thread_id_(tid), data_(data), level_(l) {}
				inline const date_time &		log_time() const { return log_time_; }
				inline const std::thread::id &	thread_id() const { return thread_id_; }
				inline const ref_string &		data() const { return data_; }
				inline level					get_level() const { return level_; }
			protected:
				const date_time		log_time_;
				std::thread::id		thread_id_;
				ref_string			data_;
				level				level_;
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
			level			level_ = info;
			bool			pass_to_parent_ = false;
		};
	}

	namespace internal  {
		class log_context
		{
		public:
			log_context() {}
			~log_context() {}

			log::logger *  get_current_logger() { return logger_; }
			void           set_current_logger(log::logger &  l) { logger_ = &l; }

			std::string &  get_cache_str() { return cache_str_; }
		protected:
			log::logger *   logger_ = nullptr;
			std::string     cache_str_;
		};
	}
}

#endif//ARA_LOG_CONTEXT_H_201602
