
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

namespace ara {

	namespace internal {

		//////////////////////////////////////////////////////

		template<class... _Types>
		class result_holder {
		public:
			typedef std::tuple<_Types...>	typeResult;

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
				if (exception_ptr_)
					exception_ptr_->rethrow();
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

			void	set_result(typeResult && r) { 
				std::unique_lock<std::mutex>		_guard(lock_);

				if (!result_ok_) {
					if (func_ptr_)
						func_ptr_->invoke_tuple(std::move(r));
					else
						result_ = std::move(r);

					result_ok_ = true;
					cond_.notify_all();
				}
			}

			void	set_func(std::shared_ptr<func_holder_base> pFunc) {
				std::unique_lock<std::mutex>		_guard(lock_);
				if (!result_ok_)
					func_ptr_ = pFunc;
				else
					pFunc->invoke_tuple( std::move(result_) );
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

			template<class exp>
			void	throw_exception(exp & e) {
				std::unique_lock<std::mutex>		_guard(lock_);
				exception_ptr_.reset(new exception_holder<exp>(e));
				result_ok_ = true;
			}

			bool				result_ok_ = false;
			std::mutex			lock_;
			std::condition_variable	cond_;
			typeResult			result_;
			std::shared_ptr<func_holder_base>	func_ptr_;
			std::unique_ptr<exception_holder_base>	exception_ptr_;
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

		// functions called by the callee
		inline const async_result &	set(typeResult && r) const {
			res_holder_ptr_->set_result(std::move(r));
			return *this;
		}

		template<class... _Result>
		inline const async_result & set(_Result... res) const {
			res_holder_ptr_->set_result(std::make_tuple(std::forward<_Result>(res)...));
			return *this;
		}

		template<class exception>
		inline void throw_exception(exception & e) const {
			res_holder_ptr_->throw_exception(e);
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
				res_.move_result( func_( std::forward<_Types>(args)... ) );
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
				res_.set(func_(std::forward<_Types>(args)...));
			}
		};

		template<typename FunCall
				,typename RetType = typename function_traits<FunCall>::result_type
				,typename = typename std::enable_if<std::is_base_of<internal::async_result_base,RetType>::value>::type
			>
		RetType	then(FunCall && f) {

			typedef func_holder<RetType, FunCall>	holder_type;
			std::shared_ptr<holder_type>	pHolder = std::make_shared<holder_type>(std::move(f));

			res_holder_ptr_->set_func(pHolder);

			return pHolder->res_;
		}
		template<typename FunCall
			, typename RetType = typename function_traits<FunCall>::result_type
			, typename = typename std::enable_if<!std::is_base_of<internal::async_result_base, RetType>::value>::type
			>
		async_result<RetType>	then(FunCall && f) {

			typedef func_holder_raw_ret<RetType, FunCall>	holder_type;
			std::shared_ptr<holder_type>	pHolder = std::make_shared<holder_type>(std::move(f));

			res_holder_ptr_->set_func(pHolder);

			return pHolder->res_;
		}


		inline void	move_result(async_result p) const {
			res_holder_ptr_->set_result(std::move(p.res_holder_ptr_->result_));
		}
	protected:

		std::shared_ptr<typeHolder>		res_holder_ptr_;
	};
}

#endif//ARA_PROMISE_H

