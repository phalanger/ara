#ifndef ARA_LOG_APPENDER_STDSTREAM_H_201603
#define ARA_LOG_APPENDER_STDSTREAM_H_201603

#include "log_imp.h"
#include <iostream>

namespace ara {
	namespace log {

		class default_log_format
		{
		public:
			static inline void output(std::ostream & out, const log_data & data, const char * lpTimeFormat) {
				stream_printf(out).printf("T:%v [%v] (%v) %v: "
					, data.thread_id()
					, data.log_time().format(lpTimeFormat)
					, data.get_logger().get_name()
					, log_data::get_level_name(data.get_level())
					);
			}
		};

		class appender_stdstream : public appender
		{
		public:
			appender_stdstream(std::ostream  &	out) : out_(out) {}

			virtual bool		before_write(const log_data & data, std::ostream & out) { 
				default_log_format::output(out, data, nullptr);
				return true; 
			}
			virtual bool		on_flush(const log_data & data, const ref_string & content) {
				std::lock_guard<std::mutex>		_guard(lock_);
				out_ << content << std::flush;
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
