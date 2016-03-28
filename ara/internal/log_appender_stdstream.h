#ifndef ARA_LOG_APPENDER_STDSTREAM_H_201603
#define ARA_LOG_APPENDER_STDSTREAM_H_201603

#include "log_imp.h"
#include <iostream>

namespace ara {
	namespace log {

		class appender_stdstream : public appender
		{
		public:
			appender_stdstream(std::ostream  &	out) : out_(out) {}

			virtual bool		onWrite(const log_data & data, const logger & l, std::string & cache_str) {
				std::lock_guard<std::mutex>		_guard(lock_);
				out_ << "T:" << data.thread_id() << " [" << data.log_time().format() << "] (" <<  l.get_name() << ')' << log_data::get_level_name(data.get_level()) << ": " << data.data() << std::endl;
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
