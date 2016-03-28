
#ifndef ARA_LOG_H
#define ARA_LOG_H

#include "ara_def.h"
#include "internal/log_imp.h"

namespace ara {

	class glog : public log::log_imp
	{
	public:
		glog(const char * name = nullptr) : log::log_imp(nullptr) {
			init_logger(name);
		}

		bool can_display(log::level l) const {
			return get_logger() && get_logger()->can_display(l);
		}

		~glog() {
			thread_context::get()._get_log_context().set_current_logger(ori_logger_);
		}

		static log::logger &		get_logger_by_name(const char * name = nullptr) {
			if ( LIKELY(name == nullptr) ) {
				auto & context = thread_context::get()._get_log_context();
				auto p = context.get_current_logger();
				if (UNLIKELY(p == nullptr)) {
					p = log::logger_mgr::get().get_root();
					context.set_current_logger(p);
				}
				return *p;
			}
			return *(log::logger_mgr::get().get_logger(name));
		}

		static void dummy() {}

	protected:
		void		init_logger(const char * name) {
			auto & context = thread_context::get()._get_log_context();
			ori_logger_ = context.get_current_logger();
			if ( UNLIKELY(ori_logger_ == nullptr) ) {
				ori_logger_ = log::logger_mgr::get().get_root();
				context.set_current_logger(ori_logger_);
			}
			if (UNLIKELY(name != nullptr)) {
				logger_ = log::logger_mgr::get().get_logger(name);
				context.set_current_logger(logger_);
			} else
				logger_ = ori_logger_;
		}

		log::logger	*		ori_logger_ = nullptr;
	};

#define	LOG_DEBUG(...)		if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::debug))		glog::dummy(); else glog( __VA_ARGS__ )(log::debug)
#define	LOG_INFO(...)		if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::info))		glog::dummy(); else glog( __VA_ARGS__ )(log::info)
#define	LOG_NOTICE(...)		if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::notice))		glog::dummy(); else glog( __VA_ARGS__ )(log::notice)
#define	LOG_WARNING(...)	if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::warning))		glog::dummy(); else glog( __VA_ARGS__ )(log::warning)
#define	LOG_ERROR(...)		if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::error))		glog::dummy(); else glog( __VA_ARGS__ )(log::error)
#define	LOG_CRITICAL(...)	if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::critical))	glog::dummy(); else glog( __VA_ARGS__ )(log::critical)
#define	LOG_ALERT(...)		if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::alert))		glog::dummy(); else glog( __VA_ARGS__ )(log::alert)
#define	LOG_EMERGENCY(...)	if (!glog::get_logger_by_name( __VA_ARGS__ ).can_display(log::emergency))	glog::dummy(); else glog( __VA_ARGS__ )(log::emergency)

	int example() {

		glog	g;
		g(log::debug).printfln("Debug log");

		LOG_INFO().printf("Hello");
		LOG_INFO("App.Server").printf("Hello");
	}


}

#endif // ARA_LOG_H

