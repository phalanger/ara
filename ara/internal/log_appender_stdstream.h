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
