
#ifndef ARA_PROMISE_H
#define ARA_PROMISE_H

#include "utils.h"
#include "datetime.h"

#include <functional>
#include <tuple>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <type_traits>
#include <exception>

/*
	//Define the result data structure
	ara::async_result<int, bool, std::string>	result;

	//Do something asynchronously
	ara::make_thread([result](){

		//Do someting
		foo();
		//...

		//then set the result:
		result.set( 100, false, "some string result...");

		//Or throw exception, like one of these:
		result.throw_exception( std::bad_exception() );
		result.throw_exception_ptr( std::current_exception() );
		result.throw_current_exception();

		//Or handle
	});

	//and then, caller can wait for the result synchronously
	result.wait();

	//or wait for result with timeout setting
	result.wait_from_now( 10_s );
	//or
	result.wait_until( std::chrono::system_clock::now() + 10_s );

	//Or check the result is whether ready
	if (result.ready()) {
		//Do something
	}
	//Or check whether has exception
	if (result.has_exception()) {
		try {
			result.rethrow();		// throw the exceptioon again
		}catch( MyException & e) {
			//.,..
		}catch( std::exception & e2) {
			//...
		}catch( ... ) {
			//...
		}

	}

	//Or get the result value(s) directly without wait/wait_xxxx() or after wait/wait_xxxx()
	int a = result.get();	//default get the first element of result. and get() return value until the result is ready.
	bool b = result.get<1>();
	std::string c = std::move( result.get<2>() );

	//Or the caller get the result asynchronously
	result.on_exception([](std::exception_ptr ptr){
		try {
			std::rethrow_exception(ptr);
		}catch( MyException & e) {
			//.,..
		}catch( std::exception & e2) {
			//...
		}catch( ... ) {
			//...
		}
	}).then([](int a, bool b, const std::string & s){
		//Do something with the result
	});

	//tehn() can chain all the async_result step by step
	result.then([](int a, bool b, const std::string & c) -> ara::async_result<int, double> {
		//Do something with the result
		ara::async_result<int, double> r;
		// do something asynchronously and set the result

		return r;
	}).then([](int d, double e) -> ara::async_result<int, int> {
		ara::async_result<int, int> r;
		// do something asynchronously and set the result
		return r;
	}).then([](int f, int g) -> double {
		double d = 1.0;
		return d;
	}).then([](double h){
	
	});

	//Important:
	// should define the on_exception() callback before then() callback

  */

namespace ara {

	namespace internal {

		//////////////////////////////////////////////////////

		typedef std::function<void(std::exception_ptr ptr)>		typeFuncExceptionCallback;

		template<class... _Types>
		class result_holder {
		public:
			typedef std::tuple<_Types...>	typeResult;

			result_holder() = default;
			result_holder(typeResult && init) : result_(std::move(init)) {}

			struct func_holder_base {
				virtual ~func_holder_base() {}
				virtual void invoke(_Types&&... args) = 0;

				template <class Tuple, std::size_t... I>
				inline void invoke_tuple_impl(Tuple &&t, ara::index_sequence<I...>) {
					invoke(std::get<I>(std::forward<Tuple>(t))...);
				}
				inline void invoke_tuple(typeResult && res) {
					std::size_t constexpr tSize = std::tuple_size<typename std::remove_reference<typeResult>::type>::value;
					invoke_tuple_impl(std::move(res), ara::make_index_sequence<tSize>());
				}
			};

			void	wait() {
				std::unique_lock<std::mutex>		_guard(lock_);
				while (!result_ok_)
					cond_.wait(_guard);
			}

			void	wait_and_handle_exception() {
				std::unique_lock<std::mutex>		_guard(lock_);
				while (!result_ok_)
					cond_.wait(_guard);
				if (exception_ptr_) {
					_guard.unlock();
					exception_ptr_->rethrow();
				}
			}

			inline bool	wait_from_now(const timer_val & t) {
				auto now = std::chrono::system_clock::now();
				auto exp = now + t.to_duration();
				return wait_until(exp);
			}

			template<class Clock, class Duration>
			bool	wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time) {
				std::unique_lock<std::mutex>		_guard(lock_);
				while (!result_ok_)
					if (cond_.wait_until(_guard, timeout_time) == std::cv_status::timeout)
						break;
				return result_ok_;
			}

			inline bool has_exception() const {
				return exception_ptr_ != nullptr;
			}

			inline bool ready() const {
				return result_ok_;
			}

			void	set_result(typeResult && r, bool atThreadExit) { 
				std::unique_lock<std::mutex>		_guard(lock_);

				if (!result_ok_) {
					if (func_ptr_) {
						std::unique_ptr<func_holder_base> tmp;
						tmp.swap(func_ptr_);

						_guard.unlock();
						result_ok_ = true;
						tmp->invoke_tuple(std::move(r));
					} else {
						result_ = std::move(r);
						result_ok_ = true;
					}
					
#if !defined(ARA_GCC_VER) || __GNUC__ > 4
					if (atThreadExit)
						std::notify_all_at_thread_exit(cond_, std::move(_guard));
					else
#endif
						cond_.notify_all();
				}
			}

			void	set_func(std::unique_ptr<func_holder_base> && pFunc) {
				std::unique_lock<std::mutex>		_guard(lock_);
				if (!result_ok_)
					func_ptr_ = std::move( pFunc );
				else {
					_guard.unlock();
					pFunc->invoke_tuple( std::move(result_) );
				}
					
			}

			class exception_holder_base {
			public:
				virtual ~exception_holder_base() {}
				virtual void rethrow() = 0;
			};
			template<class exp>
			class exception_holder : public exception_holder_base {
			public:
				exception_holder(exp && e) : e_(std::move(e)) {}
				exception_holder(const exp & e) : e_(e) {}
				virtual void rethrow() {
					throw e_;
				}
			protected:
				exp e_;
			};
			class exception_ptr_holder : public exception_holder_base {
			public:
				exception_ptr_holder(std::exception_ptr p) : p_(p) {}
				virtual void rethrow() {
					std::rethrow_exception(p_);
				}
			protected:
				std::exception_ptr p_;
			};

			template<class exp>
			void	throw_exception(exp && e) {
				std::unique_lock<std::mutex>		_guard(lock_);
				if (func_exp_) {
					try {
						result_ok_ = true;
						throw e;
					} catch (...) {
						_guard.unlock();
						func_exp_(std::current_exception());
						func_exp_ = nullptr;
					}
				} else {
					exception_ptr_.reset(new exception_holder<exp>(std::forward<exp>(e)));
					result_ok_ = true;
					if (func_ptr_) {
						std::unique_ptr<func_holder_base> tmp;
						tmp.swap(func_ptr_);

						_guard.unlock();
						tmp->invoke_tuple(std::move(result_));
					}
				}
				
				cond_.notify_all();
			}
			void throw_exception_ptr(std::exception_ptr ptr) {
				std::unique_lock<std::mutex>		_guard(lock_);
				if (func_exp_) {
					result_ok_ = true;
					_guard.unlock();
					func_exp_(ptr);
					func_exp_ = nullptr;
				}
				else {
					exception_ptr_.reset(new exception_ptr_holder(ptr));
					result_ok_ = true;
					if (func_ptr_) {
						std::unique_ptr<func_holder_base> tmp;
						tmp.swap(func_ptr_);

						_guard.unlock();
						tmp->invoke_tuple(std::move(result_));
					}
				}
				cond_.notify_all();
			}
			void throw_current_exception() {
				throw_exception_ptr(std::current_exception());
			}

			void rethrow() {
				std::unique_lock<std::mutex>		_guard(lock_);
				if (exception_ptr_) {
					_guard.unlock();
					exception_ptr_->rethrow();
				}
			}

			void on_exception(typeFuncExceptionCallback && func) {
				std::unique_lock<std::mutex>		_guard(lock_);
				if (result_ok_) {
					if (exception_ptr_) {
						try {
							exception_ptr_->rethrow();
						} catch (...) {
							_guard.unlock();
							func(std::current_exception());
						}
					}
				} else {
					func_exp_ = std::move(func);
				}
			}

			inline bool	should_stop() const {
				return should_stop_;
			}
			inline void	cancel() {
				should_stop_ = true;
			}

			std::unique_ptr<func_holder_base>	func_ptr_;
			std::unique_ptr<exception_holder_base>	exception_ptr_;
			std::mutex			lock_;
			std::condition_variable	cond_;
			typeFuncExceptionCallback	func_exp_;
			typeResult			result_;
			bool				result_ok_ = false;
			bool				should_stop_ = false;
		};

		struct async_result_base {};

	}

	template<class... _Types>
	class async_result : public internal::async_result_base
	{
	public:
		typedef internal::result_holder<_Types...>	typeHolder;
		typedef typename typeHolder::typeResult		typeResult;

		async_result(std::shared_ptr<typeHolder> p) : res_holder_ptr_(p) {}
		async_result(const async_result & p) : res_holder_ptr_(p.res_holder_ptr_) {}
		async_result() : res_holder_ptr_(std::make_shared<typeHolder>()) {}
		explicit async_result(typeResult && initValue) : res_holder_ptr_(std::make_shared<typeHolder>(std::move(initValue))) {}

		// functions called by the callee
		inline const async_result &	set(typeResult && r) const {
			res_holder_ptr_->set_result(std::move(r), false);
			return *this;
		}

		template<class... _Result>
		inline const async_result & set(_Result... res) const {
			res_holder_ptr_->set_result(std::make_tuple(std::forward<_Result>(res)...), false);
			return *this;
		}

		inline const async_result &	set_at_thread_exit(typeResult && r) const {
			res_holder_ptr_->set_result(std::move(r), true);
			return *this;
		}

		template<class... _Result>
		inline const async_result & set_at_thread_exit(_Result... res) const {
			res_holder_ptr_->set_result(std::make_tuple(std::forward<_Result>(res)...), true);
			return *this;
		}

		template<class T>
		inline const async_result & operator=(T && v) const {
			res_holder_ptr_->set_result(std::make_tuple(std::forward<T>(v)), false);
			return *this;
		}

		template<class TException>
		inline void throw_exception(TException && e) const {
			res_holder_ptr_->throw_exception(std::forward<TException>(e));
		}
		inline void throw_exception_ptr(std::exception_ptr ptr) const {
			res_holder_ptr_->throw_exception_ptr(ptr);
		}
		inline void throw_current_exception() const {
			res_holder_ptr_->throw_current_exception();
		}

		inline bool	should_stop() const {
			return res_holder_ptr_->should_stop();
		}

		// functions called by the caller
		inline void wait() const {
			res_holder_ptr_->wait();
		}

		inline bool	wait_from_now(const timer_val & t) {
			return res_holder_ptr_->wait_from_now(t);
		}

		template<class Clock, class Duration>
		bool	wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time) {
			return res_holder_ptr_->wait_until(timeout_time);
		}

		inline bool ready() const {
			return res_holder_ptr_->ready();
		}

		inline bool has_exception() const {
			return res_holder_ptr_->has_exception();
		}

		template< std::size_t I = 0>
		inline const typename std::tuple_element<I, std::tuple<_Types...> >::type & get() const {
			res_holder_ptr_->wait_and_handle_exception();
			return std::get<I, _Types...>(res_holder_ptr_->result_);
		}
		template< std::size_t I = 0>
		inline typename std::tuple_element<I, std::tuple<_Types...> >::type & get() {
			res_holder_ptr_->wait_and_handle_exception();
			return std::get<I, _Types...>(res_holder_ptr_->result_);
		}

		template<typename typeRet, class typeFunc>
		struct func_holder : public typeHolder::func_holder_base
		{
			func_holder(typeFunc && func) : func_(std::move(func)) {}

			typeFunc		func_;
			typeRet			res_;

			virtual void	invoke(_Types&&... args) {
				try {
					func_(std::forward<_Types>(args)...).link(res_);
				} catch (...) {
					res_.throw_current_exception();
				}
			}
		};

		template<class typeFunc>
		struct func_holder_void : public typeHolder::func_holder_base
		{
			func_holder_void(typeFunc && func) : func_(std::move(func)) {}

			typeFunc		func_;

			virtual void	invoke(_Types&&... args) {
				func_(std::forward<_Types>(args)...);
			}
		};

		template<typename typeRawRet, class typeFunc>
		struct func_holder_raw_ret : public typeHolder::func_holder_base
		{
			func_holder_raw_ret(typeFunc && func) : func_(std::move(func)) {}

			typedef ara::async_result<typeRawRet>	typeRet;
			typeFunc		func_;
			typeRet			res_;

			virtual void	invoke(_Types&&... args) {
				try {
					res_.set(func_(std::forward<_Types>(args)...));
				} catch (...) {
					res_.throw_current_exception();
				}
			}
		};

		template<typename linkType>
		struct link_holder : public typeHolder::func_holder_base
		{
			link_holder(linkType p) : p_(p) {}

			virtual void	invoke(_Types&&... args) {
				p_.set(std::forward<_Types>(args)...);
			}

			linkType	p_;
		};
		template<typename FunCall
				,typename RetType = typename function_traits<FunCall>::result_type
				,typename = typename std::enable_if<std::is_void<RetType>::value>::type
			>
		void	then(FunCall && f) {
			typedef func_holder_void<FunCall>	holder_type;
			std::unique_ptr<holder_type>	pHolder( new holder_type(std::move(f)) );
			res_holder_ptr_->set_func( std::move(pHolder) );
		}

		template<typename FunCall
				,typename RetType = typename function_traits<FunCall>::result_type
				,typename = typename std::enable_if<std::is_base_of<internal::async_result_base,RetType>::value>::type
			>
		RetType	then(FunCall && f) {

			typedef func_holder<RetType, FunCall>	holder_type;
			std::unique_ptr<holder_type>	pHolder( new holder_type(std::move(f)) );
			auto res = pHolder->res_;
			res_holder_ptr_->set_func( std::move(pHolder) );

			return res;
		}
		template<typename FunCall
			, typename RetType = typename function_traits<FunCall>::result_type
			, typename = typename std::enable_if<!std::is_base_of<internal::async_result_base, RetType>::value && !std::is_void<RetType>::value>::type
			>
		async_result<RetType>	then(FunCall && f) {

			typedef func_holder_raw_ret<RetType, FunCall>	holder_type;
			std::unique_ptr<holder_type>	pHolder(new holder_type(std::move(f)));
			auto res = pHolder->res_;
			res_holder_ptr_->set_func(std::move(pHolder));

			return res;
		}

		void		rethrow() const {
			res_holder_ptr_->rethrow();
		}

		async_result	on_exception(internal::typeFuncExceptionCallback && func) {
			res_holder_ptr_->on_exception(std::move(func));
			return *this;
		}

		inline void cancel() const {
			res_holder_ptr_->cancel();
		}

		template<typename linkType>
		void link(linkType res) const {
			typedef link_holder<linkType>	holder_type;
			std::unique_ptr<holder_type>	pHolder(new holder_type(res));
			res_holder_ptr_->set_func(std::move(pHolder));
			res_holder_ptr_->on_exception([res](std::exception_ptr ptr) {
				res.throw_exception_ptr(ptr);
			});
		}
	protected:

		std::shared_ptr<typeHolder>		res_holder_ptr_;
	};
}

#endif//ARA_PROMISE_H

