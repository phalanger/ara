
#ifndef ARA_THREADEXT_H
#define ARA_THREADEXT_H

#include "internal/tls_imp.h"

#include <functional>
#include <list>

namespace ara {
	namespace internal {
		struct auto_del_base {
			virtual ~auto_del_base() {}
		};
		template<class T>
		struct auto_del : public auto_del_base {
			auto_del(T * p) : p_(p) {}
			~auto_del() { delete p_; }
			T *	p_;
		};
	}

	class thread_context {
	public:
		static thread_context	& get() {
			return *internal::tls_holder<thread_context>::get();
		}

		thread_context() = default;
		thread_context(const thread_context &) = delete;
		thread_context(thread_context &&) = delete;

		~thread_context() {
			for (auto & it1 : list_run_) {
				it1();
			}
			for (auto & it2 : list_del_) {
				delete it2;
			}
		}

		void	delete_on_thread_exit(internal::auto_del_base * p) {
			list_del_.push_back(p);
		}
		template<typename T>
		void	delete_on_thread_exit(T * p) {
			list_del_.push_back( new internal::auto_del<T>(p) );
		}
		void	run_on_thread_exit(std::function<void()> && func) {
			list_run_.push_back(std::move(func));
		}
		uint64_t        next_sn()	{ return next_sn_++; }

		static void		destroy_context() {
			internal::tls_holder<thread_context>::destroy();
		}
	protected:

		std::list<internal::auto_del_base *>	list_del_;
		std::list<std::function<void()>>		list_run_;
		uint64_t								next_sn_ = 0;
	};
	/*
#define BEGIN_CALL_AUTOINFO		ara::call_begin		ARA_TMP_VAR(_call)( ara::print<ara::astring>("%v@%v#%v", ARA_FUNC_NAME, __FILE__, __LINE__) );
#define BEGIN_CALL(a)			ara::call_begin		ARA_TMP_VAR(_call)(a);

	////////////////////////////////////////////////////

	template<typename data, typename tag = void>
	class thread_local_data
	{
	public:
		thread_local_data() = default;

		data * get() const {
			data * ptr = ptr_.get();
			if (UNLIKELY(ptr == nullptr)) {
				ptr = new data();
				ptr_.reset(ptr);
			}
			return ptr;
		}

		data * operator->() const {
			return get();
		}

		data & operator*() const {
			return *get();
		}

		void reset(data * p = nullptr) {
			ptr_.reset(p);
		}

		// movable
		thread_local_data(thread_local_data&& a) {
			ptr_ = a.ptr_;
		}
		thread_local_data& operator=(thread_local_data&&a) {
			ptr_ = a.ptr_;
			return *this;
		}
	private:
		// non-copyable
		thread_local_data(const thread_local_data&) = delete;
		thread_local_data& operator=(const thread_local_data&) = delete;

		mutable thread_context_detail::thread_local_ptr<data, tag>	ptr_;
	};

	///////////////////////////////////////////////////////
	*/
}

#endif//ARA_THREADEXT_H
