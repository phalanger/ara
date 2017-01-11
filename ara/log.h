
#if 0
	int example() {

		ara::log::init_defaul_log();

		glog	g;
		g(log::debug).printfln("Debug log");

		LOG_INFO().printfln("Hello");
		LOG_INFO("App.Server").printfln("Hello");

		//Define user self log appender:
		std::stringstream	s;
		auto	appender_ptr = std::make_shared<ara::log::appender_stdstream>(s);

		//change the root append. that will display all logs to the appender
		ara::log::get_logger().set_appender(appender_ptr);
		//or
		ara::log::register_appender(appender_ptr);
		
		//change some layer appender
		ara::log::get_logger("App.Server").set_appender(appender_ptr);
		//or
		ara::log::register_appender("App.Server", appender_ptr);
		//then this log will append to the string
		LOG_INFO("App.Server").printfln("Hello");

	}
#endif//

#ifndef ARA_LOG_H
#define ARA_LOG_H

#include "ara_def.h"
#include "internal/log_imp.h"
#include "ara/internal/log_appender_stdstream.h"
#include "ara/internal/log_appender_rollingfile.h"

namespace ara {

	class glog : public log::log_imp
	{
	public:
		explicit glog(const char * name = nullptr, log::level l = log::info) : log::log_imp(nullptr) {
			init_logger(name);
			level_ = l;
		}
		explicit glog(log::level l) : log::log_imp(nullptr) {
			init_logger(nullptr);
			level_ = l;
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

		static inline void dummy() {}
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

	namespace log {
		inline void register_appender(const log::appender_ptr & p) {
			logger_mgr::get().get_root()->set_appender(p);
		}
		inline void register_appender(log::logger & logger, const log::appender_ptr & p) {
			logger.set_appender(p);
		}
		inline void register_appender(const char * name, const log::appender_ptr & p) {
			glog::get_logger_by_name(name).set_appender(p);
		}
		inline logger &		get_logger(const char * name = nullptr) {
			return glog::get_logger_by_name(name);
		}
		inline void	init_defaul_log(std::ostream & out = std::cout) {
			logger_mgr::get().get_root()->set_appender(std::make_shared<ara::log::appender_stdstream>(out));
		}
	}
}

#define	LOG_DEBUG(...)		if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::debug))		ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::debug)
#define	LOG_INFO(...)		if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::info))		ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::info)
#define	LOG_NOTICE(...)		if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::notice))	ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::notice)
#define	LOG_WARNING(...)	if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::warning))	ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::warning)
#define	LOG_ERROR(...)		if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::error))		ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::error)
#define	LOG_CRITICAL(...)	if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::critical))	ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::critical)
#define	LOG_ALERT(...)		if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::alert))		ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::alert)
#define	LOG_EMERGENCY(...)	if (!ara::glog::get_logger_by_name( __VA_ARGS__ ).can_display(ara::log::emergency))	ara::glog::dummy(); else ara::glog( __VA_ARGS__ )(ara::log::emergency)

#endif // ARA_LOG_H

