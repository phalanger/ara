
#ifndef ARA_THREADEXT_H
#define ARA_THREADEXT_H

#include "stringext.h"
#include "internal/tls_imp.h"
#include "internal/thread_state.h"
#include "internal/log_base.h"

#include <functional>
#include <list>
#include <thread>

/*
	//Thread local storage
	ara::thread_local_data_ptr<TLocalData>		a1;
	if (a1.empty()) {
		a1.reset(new TLocalData());
	}

	TLocalData & a1 = ara::thread_local_data<TLocalData>();
	TLocalData & a2 = ara::thread_local_data<TLocalData, Test2>();


	// Call stack info, user control the call stack content.
	{
		BEGIN_CALL_AUTOINFO;
		xxxxxx
	}
	{
		BEGIN_CALL("Function 1");
		xxxxxx
	}
	ara::thread_context::dump_all_thread_state(o3);
	ara::thread_context::navigate_all_thread_state([]( ara::internal::thread_state & s ){
	});

	ara::thread_context::register_after_call([](ara::internal::thread_call & call){
		ara::stream_printf(std::cout, "Finish %v, used: %v", call.get_call_info() , clock() - call.get_start_clock()) << std::endl;
	});
		
	// Create thread that auto clear the thread context
	ara::make_thread( []() {
		
	}); 

	// Or

	// Destroy thread context self
	std::thread t([](){
		//Do something
		ara::thread_context::destroy_context();
	});

*/

namespace ara {

	struct auto_del_base {
		virtual ~auto_del_base() {}
	};

	namespace internal {
		template<class T>
		struct auto_del : public auto_del_base {
			auto_del(T * p) : p_(p) {}
			~auto_del() { delete p_; }
			T *	p_;
		};
	}

	class thread_context {
	public:
		static inline thread_context	& get() {
			return *internal::tls_holder<thread_context>::get();
		}

		thread_context() {
			thread_state_.append_after(internal::thread_state::get_root());
		}
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

		void	delete_on_thread_exit(auto_del_base * p) {
			list_del_.push_back(p);
		}
		template<typename T>
		void	delete_on_thread_exit(T * p) {
			list_del_.push_back( new internal::auto_del<T>(p) );
		}
		void	run_on_thread_exit(std::function<void()> && func) {
			list_run_.push_back(std::move(func));
		}
		void	navigate_callstack(std::function<void(const internal::thread_call &)> && func) {
			_get_thread_state().navigate_callstack(std::move(func));
		}

		uint64_t        next_sn()	{ return next_sn_++; }

		static void		destroy_context() {
			internal::tls_holder<thread_context>::destroy();
		}

		static void		dump_all_thread_state(std::ostream & out) {
			internal::thread_state::dump_all(out);
		}
		static void		navigate_all_thread_state(std::function<void (internal::thread_state &)> && func) {
			internal::thread_state::navigate_all(std::move(func));
		}
		static void		register_after_call(std::function<void(internal::thread_call &)> && func) {
			internal::thread_state::register_after_call(std::move(func));
		}
	public:
		//internal interface

		template<class T>
		internal::thread_local_ptr<T> * _get_local_storage(int idx) {
			return tls_mgr_.template get<T>(idx);
		}

		internal::thread_state	& _get_thread_state() { 
			return thread_state_; 
		}

		static void	_global_init() {
			internal::thread_state::get_root().as_root();
		}

		inline log::log_context	 & _get_log_context() {
			return	log_context_;
		}
	protected:
		std::list<auto_del_base *>	list_del_;
		std::list<std::function<void()>>		list_run_;
		uint64_t								next_sn_ = 0;
		internal::thread_local_storage			tls_mgr_;
		internal::thread_state					thread_state_;
		log::log_context						log_context_;
	};

	////////////////////////////////////////////////////

	template<typename data, typename tag = void>
	class thread_local_data_ptr
	{
	public:
		thread_local_data_ptr() = default;

		inline data * get() const {
			_check();
			return ptr_->get();
		}

		inline data * operator->() const {
			return get();
		}

		inline data & operator*() const {
			return *get();
		}

		void reset(data * p = nullptr) {
			_check();
			ptr_->reset(p);
		}

		bool empty() const {
			if (id_ == -1)
				return true;
			_check();
			return ptr_ == nullptr || ptr_->get() == nullptr;
		}

		bool operator==(const data * p) const {
			if (id_ == -1)
				return p == nullptr;
			else if (ptr_ == nullptr)
				return p == nullptr;
			return ptr_.get() == p;
		}

	private:
		inline void		_check() const {
			if (UNLIKELY(id_ == -1)) {
				std::call_once(init_flag_, []() {
					id_ = internal::thread_local_id_manager<int>::get().getid();
				});
			}
			if (UNLIKELY(ptr_ == nullptr)) {
				ptr_ = thread_context::get().template _get_local_storage<data>(id_);
			}
		}

		// non-copyable
		thread_local_data_ptr(const thread_local_data_ptr&) = delete;
		thread_local_data_ptr& operator=(const thread_local_data_ptr&) = delete;
		thread_local_data_ptr(thread_local_data_ptr&& a) = delete;

		mutable internal::thread_local_ptr<data>	*	ptr_ = nullptr;
		static std::once_flag init_flag_;
		static volatile int		id_;
	};

	template<typename data, typename tag>
	std::once_flag thread_local_data_ptr<data,tag>::init_flag_;
	template<typename data, typename tag>
	volatile int thread_local_data_ptr<data, tag>::id_ = -1;

	template<typename data,typename tag = void>
	data &	thread_local_data() {
		thread_local_data_ptr<data, tag>	p;
		if (p.empty())
			p.reset(new data);
		return *p;
	}

	//////////////////////////////////////////////////////////////////////////
			
	class call_begin : public internal::thread_call
	{
	public:
		typedef internal::thread_call	typeParent;
		call_begin(const char * p, clock_t s = 0, typeParent::ext_data * pExt = nullptr)
			: typeParent(p, s, pExt), ts_(thread_context::get()._get_thread_state()) {
			ts_.push(this);
		}
		call_begin(const std::string & p, clock_t s = 0, typeParent::ext_data * pExt = nullptr)
			: typeParent(p, s, pExt), ts_(thread_context::get()._get_thread_state()) {
			ts_.push(this);
		}
		call_begin(std::string && p, clock_t s = 0, typeParent::ext_data * pExt = nullptr)
			: typeParent(p, s, pExt), ts_(thread_context::get()._get_thread_state()) {
			ts_.push(this);
		}
		~call_begin() {
			ts_.pop();
		}
	protected:
		internal::thread_state	&	ts_;
	};

	//////////////////////////////////////////////////////////////////////////

	template < typename _Callable, typename... _Args>
	std::thread		make_thread(_Callable && f, _Args&&... args) {
		auto func = std::bind<void>(std::forward<_Callable>(f), std::forward<_Args>(args)...);
		std::thread res([func = std::move(func)](){
			func();
			thread_context::destroy_context();
		});
		return res;
	}
}

#define ARA_JOIN_2(a, b)			a##b
#define ARA_JOIN_1(a, b)			ARA_JOIN_2(a, b)
#define ARA_TMP_VAR(a)				ARA_JOIN_1(a, __LINE__)

#define BEGIN_CALL_AUTOINFO		ara::call_begin		ARA_TMP_VAR(_call)( ara::str_printf<std::string>("%v@%v#%v", ARA_FUNC_NAME, __FILE__, __LINE__) )
#define BEGIN_CALL(...)			ara::call_begin		ARA_TMP_VAR(_call)( __VA_ARGS__)

#endif//ARA_THREADEXT_H
