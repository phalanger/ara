
#ifndef ARA_INTERNAL_SINGLETON_MGR_H
#define ARA_INTERNAL_SINGLETON_MGR_H

#include "../ara_def.h"
#include "../threadext.h"

namespace ara {
	namespace internal {

		template<class C>
		class singleton_mgr
		{
		public:
			singleton_mgr() {}
			~singleton_mgr() {
				for (auto & it1 : list_run_) {
					it1();
				}
				for (auto & it2 : list_del_) {
					delete it2;
				}
			}

			static singleton_mgr & get() {
				if (UNLIKELY(instance_ == nullptr)) {
					std::call_once(init_flag, []() {
						if (!instance_) {
							std::unique_ptr<singleton_mgr>	_au(new singleton_mgr);
							instance_ = _au.release();
						}
						std::atexit(destroy);
					});
				}
				return *instance_;
			}

			template<typename T>
			void	delete_on_app_exit(T * p) {
				list_del_.push_back(new auto_del<T>(p));
			}
			void	run_on_thread_exit(std::function<void()> && func) {
				list_run_.push_back(std::move(func));
			}
		protected:
			static void destroy() {
				delete instance_;
			}

			std::list<auto_del_base *>			list_del_;
			std::list<std::function<void()>>	list_run_;
			static singleton_mgr *		instance_;
			static std::once_flag init_flag;
		};

		template<class C>
		singleton_mgr<C> * singleton_mgr<C>::instance_ = nullptr;
		template<class C>
		std::once_flag singleton_mgr<C>::init_flag;
	}

}

#endif//ARA_INTERNAL_SINGLETON_MGR_H
