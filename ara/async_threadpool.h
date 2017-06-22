
#ifndef ARA_ASYNC_THREADPOOL_H
#define ARA_ASYNC_THREADPOOL_H

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include "threadext.h"
#include "stringext.h"

#include <memory>

namespace ara {

	class async_thread_pool;
	typedef std::shared_ptr<async_thread_pool>		async_thread_pool_ptr;

	class async_thread_pool : public std::enable_shared_from_this<async_thread_pool>
	{
	public:
		static async_thread_pool_ptr	make_thread_pool(const std::string & strName) {
			return std::make_shared<async_thread_pool>(strName);
		}

		async_thread_pool(const std::string & strName) : name_(strName) {}
		~async_thread_pool() {
			stop();
		}

		async_thread_pool &	init(size_t nThreadCount, size_t nStackSize = 1024 * 1024) {
			thread_count_ = nThreadCount;
			stack_size_ = nStackSize;
			return *this;
		}

		const std::string &	name() const {
			return name_;
		}

		void	start() {
			if (workder_)
				stop();

			workder_.reset(new boost::asio::io_service::work(io_));
			boost::thread::attributes attrs;
			attrs.set_stack_size(stack_size_);
			io_.reset();

			for (size_t i = 0; i < thread_count_; ++i)	{

				std::unique_ptr<boost::thread>	task(new boost::thread(attrs, [this, i]() {

					defer		_au([]() {thread_context::destroy_context(); });
					while (!io_.stopped())
					{
						try
						{
							std::string		strInfo = ara::printf<std::string>("Pool[%v] ID:%d Index:%d", name_, boost::this_thread::get_id(), i);
							BEGIN_CALL(strInfo.c_str());

							io_.run();
						}
						catch (...)
						{
							if (func_exception_)
								func_exception_(std::current_exception());
						}
					}

				}));
				threads_.add_thread(task.get());
				task.release();
			}
		}
		void	stop() {
			if (!workder_)
				return;
			workder_.reset();
			io_.stop();
			threads_.join_all();
		}

		inline boost::asio::io_service	&	io() { return io_; }

		template<typename handle>
		async_thread_pool &	post(handle && func) {
			io_.post( std::move(func) );
			return *this;
		}
		template<typename handle>
		async_thread_pool &	dispatch(handle && func) {
			io_.dispatch( std::move(func) );
			return *this;
		}

		template<typename handle>
		async_thread_pool &	on_exception(handle && h) {
			func_exception_ = std::move(h);
			return *this;
		}

		/*
		template< class Function, class... Args>
		std::future<typename std::result_of<Function(Args...)>::type>
			async(Function&& f, Args&&... args)
		{
			typedef typename std::result_of<Function(Args...)>::type	retType;
			std::packaged_task<retType()>	task(std::bind(std::move(f), std::forward<Args>(args)...));
			auto res = task.get_future();

			async::asio_post(io_, std::move(task));
			return res;
		}
		*/
	protected:
		std::string					name_;
		boost::thread_group			threads_;
		size_t						thread_count_ = 0;
		size_t						stack_size_ = 1024 * 1024;
		std::shared_ptr<boost::asio::io_service::work>		workder_;
		boost::asio::io_service		io_;
		std::function<void(std::exception_ptr)>	func_exception_;
	};

}


#endif//ARA_ASYNC_THREADPOOL_H
