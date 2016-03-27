
#ifndef ARA_SINGLETON_H
#define ARA_SINGLETON_H

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
					if (!instance_)
						instance_ = new T;
					std::atexit(destroy);
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
				std::atexit(destroy);
			});
			return instance_ == p;
		}
	protected:
		static void	destroy() {
			delete instance_;
		}
		static T *		instance_;
		static std::once_flag init_flag;
	};

	template<class T,class tag>
		T * singleton<T, tag>::instance_ = nullptr;
	template<class T, class tag>
		std::once_flag singleton<T, tag>::init_flag;
}

#endif//ARA_SINGLETON_H
