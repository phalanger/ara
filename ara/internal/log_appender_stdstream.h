#ifndef ARA_LOG_APPENDER_STDSTREAM_H_201603
#define ARA_LOG_APPENDER_STDSTREAM_H_201603

#include "log_imp.h"
#include <iostream>

namespace ara {
	namespace log {

		class default_log_format
		{
		public:
			static void output(const log_data & data, const logger & l, const char * lpTimeFormat, std::string & cache_str) {
				cache_str.clear();
				cache_str.reserve( data.data().length() + 64 );
				strext(cache_str).printf("T:%v [%v] (%v) %v: %v", data.thread_id(), data.log_time().format(lpTimeFormat), l.get_name(), log_data::get_level_name(data.get_level()), data.data());
			}
		};

		template<class format = default_log_format>
		class appender_stdstream : public appender
		{
		public:
			appender_stdstream(std::ostream  &	out) : out_(out) {}

			virtual bool		onWrite(const log_data & data, const logger & l, std::string & cache_str) {
				format::output(data, l, nullptr, cache_str);
				std::lock_guard<std::mutex>		_guard(lock_);
				out_ << cache_str;
				return true;
			}

			virtual std::string	dump_setting() const {
				return "stdstream_appender";
			}

			virtual void	flush_all() {
				out_ << std::flush;
			}
		protected:
			std::mutex	lock_;
			std::ostream  &	out_;
		};
	}
}

#endif//ARA_LOG_APPENDER_STDSTREAM_H_201603
