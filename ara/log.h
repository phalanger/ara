
#ifndef ARA_LOG_H
#define ARA_LOG_H

#include "ara_def.h"
#include "internal/log_imp.h"

namespace ara {

	class gLog : public log::log_imp
	{
	public:
		gLog(const char * Name = nullptr) : log::log_imp(get_logger(Name)) {}

		bool can_display(log::level l) const {
			return logger_ && logger_->can_display(l);
		}

		static log::logger *		get_default_logger() {
			log::logger * p = thread_context::get()._get_log_context().get_current_logger();
			if (p == nullptr)
				return log::logger_mgr::get().get_root();
			return p;
		}
		static log::logger *		get_logger(const char * name = nullptr) {
			if (name == nullptr || *name == '\0')
				return get_default_logger();
			return log::logger_mgr::get().get_logger(name);
		}

		static void dummy() {}
	};

#define	LOG_INFO(x)		if (!gLog::get_logger(x)->can_display(log::info))	  gLog::dummy(); else gLog(x)(log::info)

	int func() {
		LOG_INFO().printf("aasdasdasdsa");
		LOG_INFO("App.Server").printf("aasdasdasdsa");
	}

}

#endif // ARA_LOG_H

