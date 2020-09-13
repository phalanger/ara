

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch2/catch.hpp"

//#include <atomic>

#include "ara/async_respool.h"
#include "ara/async_threadpool.h"
#include "ara/event.h"
#include "ara/log.h"
#include "test_async_helper.h"

TEST_CASE("async res pool", "[async]") {

	SECTION("base") {

		ara::log::init_defaul_log();
		auto oldLevel = ara::log::get_logger().set_level(ara::log::debug);
		ara::defer	_auGuard([oldLevel]() {
			ara::log::get_logger().set_level(oldLevel);
			});

		typedef ara::async_resourcepool<std::string>	arespool;

		auto pool = arespool::make_pool(100);
		pool->set_trace_log(true);

		pool->add("res1", 0);
		pool->add("res1", 1);
		pool->add("res1", 2);
		pool->add("res2", 3);
		pool->add("res2", 4);
		pool->add("res2", 5);
		pool->add("res3", 6);

		enum {
			THREAD1_GOT_KEY = 1,
			THREAD2_GOT_KEY,
			THREAD1_RELEASE_KEY,
			ALL_FINISHED,
		};

		ara::async_thread_pool	thread_pool("test");
		auto & io = thread_pool.io();
		thread_pool.init(2).start();

		auto errinfo = std::make_shared<async_error>();
		auto num = std::make_shared<ara::event<int>>(0);

		{
			pool->apply(io, "res1", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, arespool::async_res_token token) {
				if (ec)
					errinfo->set_error("Why not got resource res1");

				token = nullptr;
				num->signal_all(ALL_FINISHED);
			}, "test 1");
			num->wait(ALL_FINISHED);
		}

		{
			num->reset(0);
			pool->apply(io, "res4", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, arespool::async_res_token token) {

				if (ec != boost::asio::error::invalid_argument)
					errinfo->set_error("Why not got resource res4 return invalid arg");

				num->signal_all(ALL_FINISHED);
			}, "test 2");
			num->wait(ALL_FINISHED);
		}

		{
			num->reset(0);
			pool->apply(io, "res1", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, arespool::async_res_token token) {

				if (ec || token->get() != 1)
					errinfo->set_error("Get token 1 fail");
				num->signal_all(THREAD1_GOT_KEY);
				num->wait(THREAD2_GOT_KEY);
				num->signal_all(ALL_FINISHED);

			}, "test 3");

			num->wait(THREAD1_GOT_KEY);
			pool->apply(io, "res1", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, arespool::async_res_token token) {

				if (ec || token->get() != 2)
					errinfo->set_error("Get token 2 fail");

				num->signal_all(THREAD2_GOT_KEY);
			}, "test 3");

			num->wait(ALL_FINISHED);
		}

		thread_pool.stop();
	}
}
