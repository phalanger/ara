
/*
	class MyData {
	public:
		void foo();
	};

	ara::singleton<MyData>::get().foo();

*/

#ifndef ARA_SINGLETON_H
#define ARA_SINGLETON_H

#include "internal/singleton_mgr.h"

#include <mutex>
#include <atomic>

namespace ara {

	template<class T, class tag = void> 
	class singleton
	{
	public:
		static T	& get() {
			if (UNLIKELY(instance_ == nullptr)) {
				std::call_once(init_flag, []() {
					if (!instance_) {
						std::unique_ptr<T>	_au(new T);
						instance_ = _au.release();
						internal::singleton_mgr<void>::get().delete_on_app_exit(instance_);
					}
				});
			}
			return *instance_;
		}
		static bool		empty() {
			return instance_ == nullptr;
		}
		static bool		set(T * p) {
			if (!empty())
				return false;
			std::call_once(init_flag, [p]() {
				instance_ = p;
				internal::singleton_mgr<void>::get().delete_on_app_exit(instance_);
			});
			return instance_ == p;
		}
	protected:
		static T *		instance_;
		static std::once_flag init_flag;
	};

	template<class T,class tag>
		T * singleton<T, tag>::instance_ = nullptr;
	template<class T, class tag>
		std::once_flag singleton<T, tag>::init_flag;
}

#endif//ARA_SINGLETON_H
