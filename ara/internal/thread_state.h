#ifndef ARA_THREAD_STATE_H_201602
#define ARA_THREAD_STATE_H_201602

#include "../ara_def.h"
#include "../dlist.h"

#include <vector>
#include <atomic>
#include <mutex>
#include <iostream>
#include <functional>

namespace ara {
	namespace internal  {

		class thread_call
		{
		public:
			class ext_data
			{
			public:
				ext_data(int code, void * data) : code_(code), ext_data_(data) {}
				int		get_code() const { return code_; }
				void *	get_data() const { return ext_data_; }
			protected:
				int				code_;
				void *			ext_data_;
			};

			thread_call(const char * p, clock_t s = 0, ext_data * pext = nullptr)
				: ptr_next_(nullptr), ptr_call_info_(p), clock_start_(s), ptr_ext_data_(pext) {}
			thread_call(std::string && str, clock_t s = 0, ext_data * pext = nullptr)
				: ptr_next_(nullptr), ptr_call_info_(nullptr), clock_start_(s), ptr_ext_data_(pext), dummy_info_(str) {
				ptr_call_info_ = dummy_info_.c_str();
			}
			thread_call(const std::string & str, clock_t s = 0, ext_data * pext = nullptr)
				: ptr_next_(nullptr), ptr_call_info_(nullptr), clock_start_(s), ptr_ext_data_(pext), dummy_info_(str) {
				ptr_call_info_ = dummy_info_.c_str();
			}
			thread_call *	join(thread_call * p) {
				ptr_next_ = p;
				return this;
			}
			thread_call	* detach() {
				return ptr_next_;
			}
			inline const char *	get_call_info() const { return ptr_call_info_; }
			inline clock_t			get_start_clock() const { return clock_start_; }
			inline const ext_data * get_ext_data() const { return ptr_ext_data_; }
			inline const thread_call *	get_caller() const { return ptr_next_; }
		protected:
			thread_call *	ptr_next_ = nullptr;
			const char *	ptr_call_info_ = nullptr;
			clock_t			clock_start_ = 0;
			ext_data * 		ptr_ext_data_ = nullptr;
			std::string		dummy_info_;
		private:
			thread_call(const thread_call &) = delete;
			thread_call(thread_call &&) = delete;
			thread_call & operator=(const thread_call &) = delete;
			thread_call & operator=(thread_call &&) = delete;
		};

		template<typename LockType, typename Root>
		class thread_state_lock
		{
		public:
			LockType			lock_;
			static LockType		g_lock;
			static std::function<void(thread_call *)>		g_after_call;
			static Root			root_;
		};
		template<typename LockType, typename Root>
		LockType	thread_state_lock<LockType,Root>::g_lock;
		template<typename LockType, typename Root>
		std::function<void(thread_call *)>	thread_state_lock<LockType, Root>::g_after_call;
		template<typename LockType, typename Root>
		Root	thread_state_lock<LockType, Root>::root_;

		class thread_state : public dlist<thread_state>, thread_state_lock<std::mutex, thread_state>
		{
		public:
			typedef std::mutex	LockType;

			thread_state() : ptr_callstack_(nullptr) {
				id_ = std::this_thread::get_id();
			}
			~thread_state() {
				if (!alone()) {
					std::lock_guard<LockType>	_guard(g_lock);
					unlink();
				}
			}
			void	push(thread_call * p) {
				std::lock_guard<LockType>		_guard(lock_);
				ptr_callstack_ = p->join(ptr_callstack_);
			}
			thread_call *	pop() {
				if (g_after_call)
					g_after_call(ptr_callstack_);
				std::lock_guard<LockType>		_guard(lock_);
				thread_call * ret = ptr_callstack_;
				ptr_callstack_ = ptr_callstack_ ? ptr_callstack_->detach() : nullptr;
				return ret;
			}
			void	navigate_callstack(std::function<void(const thread_call *)> func) {
				std::lock_guard<LockType>		_guard(lock_);
				func(ptr_callstack_);
			}
			static	void	register_after_call(std::function<void(thread_call *)>	 f) {
				g_after_call = f;
			}
			void		dump_callstack(std::ostream & out) {
				std::lock_guard<LockType>        _guard(lock_);
				bool boFirst = true;
				for (const thread_call * p = ptr_callstack_; p; p = p->get_caller()) {
					if (boFirst)
						boFirst = false;
					else
						out << " <- ";
					const char * pInfo = p->get_call_info();
					out << pInfo;
				}
			}
			const std::thread::id	&	get_id() const {
				return id_;
			}
			static thread_state & get_root() {
				return thread_state_lock<std::mutex, thread_state>::root_;
			}
			static void	dump_all(std::ostream & out) {
				std::lock_guard<LockType>        _guard(g_lock);
				auto p = root_.get_next();
				auto pend = &root_;
				while (p != pend) {
					out << "[T:" << p->get_id() << "]: ";
					p->dump_callstack(out);
					out << std::endl;
					p = p->get_next();
				}
			}
			static void navigate_all(std::function<void(internal::thread_state &)> && func) {
				std::lock_guard<LockType>        _guard(g_lock);
				auto p = root_.get_next();
				auto pend = &root_;
				while (p != pend) {
					func(*p);
					p = p->get_next();
				}
			}
		protected:
			thread_call	*	ptr_callstack_ = nullptr;
			std::thread::id	id_;
		private:
			thread_state(const thread_state &) = delete;
			thread_state(thread_state &&) = delete;
			thread_state & operator=(const thread_state &) = delete;
			thread_state & operator=(thread_state &&) = delete;
		};
	
	}//internal
}//ara

#endif//ARA_THREAD_STATE_H_201602

