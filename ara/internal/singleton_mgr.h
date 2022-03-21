
#ifndef ARA_INTERNAL_SINGLETON_MGR_H
#define ARA_INTERNAL_SINGLETON_MGR_H

#include "../ara_def.h"
#include "../threadext.h"

namespace ara {
	namespace internal {

		////////////////////////////////////////////////////////////////

		class auto_destroy_func : public auto_del_base
		{
		public:
			auto_destroy_func(std::function<void()> && func) : func_(std::move(func)) {}
			~auto_destroy_func() {
				if(func_)
					func_();
			}
		protected:
			std::function<void()> func_;
		};

		////////////////////////////////////////////////////////////////

		template<class C>
		class singleton_mgr
		{
		public:
			singleton_mgr() noexcept {}
			~singleton_mgr() {
				std::unique_lock<std::mutex>	_guard(lock_);
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
				std::unique_lock<std::mutex>	_guard(lock_);
				list_del_.push_back(new auto_del<T>(p));
			}

			void	add_to_delete_list(auto_del_base * t) {
				std::unique_lock<std::mutex>	_guard(lock_);
				list_del_.push_back(t);
			}
		protected:
			static void destroy() {
				delete instance_;
			}

			std::mutex							lock_;
			std::list<auto_del_base *>			list_del_;
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
