
#ifndef ARA_INTERNAL_TLS_IMP_H
#define ARA_INTERNAL_TLS_IMP_H

#include "../ara_def.h"
#include <cstdlib>
#include <mutex>

#ifdef ARA_WIN32_VC_VER
#else//ARA_WIN32_VC_VER
	#include <pthread.h>
#endif//ARA_WIN32_VC_VER

namespace ara { namespace internal {

#ifdef ARA_WIN32_VC_VER

	template<typename T>
	class tls_holder {
	public:
		static T *	get() {
			static __declspec(thread)	T *	g_pContext = nullptr;

			if ( UNLIKELY(g_pContext == nullptr) )
				g_pContext = new T;

			static std::once_flag init_flag;
			std::call_once(init_flag, []() {
				std::atexit(destroy);
			});
			return g_pContext;
		}

		static void	destroy() {
			delete get();
		}
	};

	
#else//ARA_WIN32_VC_VER

	template<typename T>
	class tls_holder {
	public:
		static void at_main_exit() {
			delete get();
		}

		static void		make_key() {
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
			if (!key)
				return;
			T * ptr = reinterpret_cast<T *>(pthread_getspecific(key));
			pthread_setspecific(key, nullptr);
			delete ptr;
		}

		static pthread_key_t key;
		static pthread_once_t key_once;
	};

	template<typename T>
	pthread_key_t tls_holder<T>::key = 0;
	template<typename T>
	pthread_once_t tls_holder<T>::key_once = PTHREAD_ONCE_INIT;

#endif//ARA_WIN32_VC_VER

} }

#endif//ARA_INTERNAL_TLS_IMP_H
