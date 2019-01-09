
#ifndef ARA_INTERNAL_TLS_IMP_H
#define ARA_INTERNAL_TLS_IMP_H

#include "../ara_def.h"
#include <cstdlib>
#include <mutex>
#include <vector>
#include <atomic>

#ifdef ARA_WIN32_VC_VER
#else//ARA_WIN32_VC_VER
	#include <pthread.h>
#endif//ARA_WIN32_VC_VER

namespace ara {
	namespace internal {

#ifdef ARA_WIN32_VC_VER

		template<typename T>
		class tls_holder {
		public:
			static T *	get(bool toDel = false) {
				static __declspec(thread)	T *	g_pContext = nullptr;

				static std::once_flag init_flag;
				std::call_once(init_flag, []() {
					T::_global_init();
					std::atexit(destroy);
				});

				if (UNLIKELY(toDel)) {
					if (g_pContext == nullptr)
						return nullptr;
					else {
						T * res = g_pContext;
						g_pContext = nullptr;
						return res;
					}
				}
				else if (UNLIKELY(g_pContext == nullptr)) {
					g_pContext = new T;
				}

				return g_pContext;
			}

			static void	destroy() {
				delete get(true);
			}
		};


#else//ARA_WIN32_VC_VER

		template<typename T>
		class tls_holder {
		public:
			static void at_main_exit() {
				//delete get();
				destroy();
			}

			static void		make_key() {
				T::_global_init();
				pthread_key_create(&key, nullptr);
				std::atexit(at_main_exit);
			}

			static T *	get() {
				(void)pthread_once(&key_once, make_key);
				T * ptr = reinterpret_cast<T *>(pthread_getspecific(key));

				if (UNLIKELY(ptr == nullptr)) {
					std::unique_ptr<T>		p(new T);
					(void)pthread_setspecific(key, p.get());
					ptr = p.release();
				}
				return ptr;
			}
			static void	destroy() {
				if (key == pthread_key_t(-1))
					return;
				T * ptr = reinterpret_cast<T *>(pthread_getspecific(key));
				pthread_setspecific(key, nullptr);
				delete ptr;
			}

			static pthread_key_t key;
			static pthread_once_t key_once;
		};

		template<typename T>
		pthread_key_t tls_holder<T>::key = pthread_key_t(-1);
		template<typename T>
		pthread_once_t tls_holder<T>::key_once = PTHREAD_ONCE_INIT;

#endif//ARA_WIN32_VC_VER

		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		class thread_local_id_manager
		{
		public:
			thread_local_id_manager() : local_idx_() {}
			T    getid() {
				return local_idx_++;
			}
			static thread_local_id_manager & get() { return g_local_id_mgr; }
		protected:
			static thread_local_id_manager    g_local_id_mgr;
			std::atomic<T>		local_idx_;
		};

		template<typename T>
		thread_local_id_manager<T> thread_local_id_manager<T>::g_local_id_mgr;

		//////////////////////////////////////////////////////////////////////////

		class thread_local_ptr_base
		{
		public:
			virtual ~thread_local_ptr_base() {}
		};
		template<typename T>
		class thread_local_ptr : public thread_local_ptr_base
		{
		public:
			thread_local_ptr() = default;
			thread_local_ptr(const thread_local_ptr &) = delete;
			thread_local_ptr(thread_local_ptr && p) : data_ptr_(p.data_ptr_) {
				p.data_ptr_ = nullptr;
			}
			~thread_local_ptr() {
				delete data_ptr_;
			}
			T	* get() const {
				return data_ptr_;
			}
			void	reset(T * p) {
				delete data_ptr_;
				data_ptr_ = p;
			}
		protected:
			T    *  data_ptr_ = nullptr;
		};

		class thread_local_storage
		{
		public:
			thread_local_storage() noexcept {}
			thread_local_storage(const thread_local_storage &) = delete;
			thread_local_storage(thread_local_storage &&) = delete;

			~thread_local_storage() {
				for (auto it : ar_local_ptr_)
					delete it;
			}

			template<typename local_data>
			thread_local_ptr<local_data> *    get(int idx) {
				if (UNLIKELY(ar_local_ptr_.size() <= static_cast<size_t>(idx)))
					ar_local_ptr_.resize(static_cast<size_t>(idx + 1));
				if (UNLIKELY(ar_local_ptr_[idx] == nullptr))
					ar_local_ptr_[idx] = new thread_local_ptr<local_data>;
				return reinterpret_cast<thread_local_ptr<local_data> *>(ar_local_ptr_[idx]);
			}
		protected:
			typedef std::vector<thread_local_ptr_base *>    type_local_ptr_ary;
			type_local_ptr_ary            ar_local_ptr_;
		};

	}
}

#endif//ARA_INTERNAL_TLS_IMP_H
