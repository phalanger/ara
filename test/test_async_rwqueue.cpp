

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch/single_include/catch.hpp"

//#include <atomic>

#include "ara/async_rwqueue.h"
#include "ara/async_threadpool.h"
#include "ara/event.h"
#include "ara/log.h"
#include "test_async_helper.h"

TEST_CASE("async rwqueue", "[async]") {

	SECTION("base") {

		ara::log::init_defaul_log();
		auto oldLevel = ara::log::get_logger().set_level(ara::log::warning);
		ara::defer	_auGuard([oldLevel]() {
			ara::log::get_logger().set_level(oldLevel);
		});

		typedef ara::async_rwqueue<std::string>	name_queue;

		auto p = name_queue::make_rwqueue(10);
		auto num = std::make_shared<ara::event<int>>(0);

		ara::async_thread_pool	pool("test");
		auto & io = pool.io();
		pool.init(2).start();

		enum {
			THREAD1_GOT_READ_KEY = 1,
			THREAD2_GOT_WRITE_KEY_FAIL,
			THREAD1_RELEASE_READ_KEY,
			ALL_FINISHED,
		};

		auto errinfo = std::make_shared<async_error>();

		io.post([p, num, &io, errinfo]() {

			LOG_INFO().printfln("T1: begin to read work");

			p->apply_read(io, "key1", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);

				g(ara::log::info).printfln("T1: I got the read key");
				num->signal_all(THREAD1_GOT_READ_KEY);
				if (token == nullptr)
					errinfo->set_error("Token is null while T1 apply read");
				else if (!num->wait(THREAD2_GOT_WRITE_KEY_FAIL))
					errinfo->set_error("T1 fail to wait for thread2 got write key fail");
				else {
					g(ara::log::info).printfln("T1: T2 get write key fail, I will release the read key");
					token = nullptr;
				}
				num->signal_all(THREAD1_RELEASE_READ_KEY);

			}, "thread 1 apply 1");
		});

		io.post([p, num, &io, errinfo]() {

			ara::glog		g(ara::log::info);

			g.printfln("T2: begin to work wait for T1");
			REQUIRE(num->wait(THREAD1_GOT_READ_KEY));
			g.printfln("T2: T1 got the read key, now to my turn to try get the read key.");

			p->apply_read(io, "key1", ara::timer_val(1, 0), [num, &io, p, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);
				g.printfln("T2: I should got the read key.");
				if (token == nullptr) {
					errinfo->set_error("T2 should got the read key because T1 just hold read key.");
					return;
				}
				g.printfln("T2: Now I release the read key, which hold by me.");
				token = nullptr;

				p->apply_write(io, "key1", ara::timer_val(0, 1), [num, &io, p, errinfo](const boost::system::error_code & ec, ara::async_token token) {

					ara::glog		g(ara::log::info);
					g.printfln("T2: I got the write key fail, because T1 hold read key");
					if (token != nullptr) {
						errinfo->set_error("T2 should not got the write key because T1 hold read key. But now T2 got it.");
					}
					else if (ec != boost::asio::error::timed_out) {
						errinfo->set_error("T2 should not got the write key and should be timeout. But now T2 is not timeout.");
					}
					else {
						g.printfln("T2: I tell T1 to release the read key");
						num->signal_all(THREAD2_GOT_WRITE_KEY_FAIL);

						num->wait(THREAD1_RELEASE_READ_KEY);
						g(ara::log::info).printfln("T2: T1 has release read key, I will try to get the write key again");

						io.post([p, num, &io, errinfo]() {
							p->apply_write(io, "key1", ara::timer_val(1, 0), [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {
								ara::glog		g(ara::log::info);
								if (token == nullptr) {
									errinfo->set_error( ara::printf<std::string>("T2 should got write key after T1 release read key. But T2 can not got it now: %v", ec.message()));
								}
								else {
									g(ara::log::info).printfln("T2: I got the write key now");
								}
								num->signal_all(ALL_FINISHED);
							}, "thread 2 apply write key 2");
						});
					}
				}, "thread 2 apply read key 1");
			}, "thread 2 apply read key 2");
		});

		num->wait(ALL_FINISHED);
		pool.stop();

		REQUIRE(errinfo->get_error() == "");
	}

	SECTION("base 2") {

		ara::log::init_defaul_log();
		auto oldLevel = ara::log::get_logger().set_level(ara::log::warning);
		ara::defer	_auGuard([oldLevel]() {
			ara::log::get_logger().set_level(oldLevel);
		});

		typedef ara::async_rwqueue<std::string>	name_queue;

		auto p = name_queue::make_rwqueue(10);
		auto num = std::make_shared<ara::event<int>>(0);

		ara::async_thread_pool	pool("test");
		auto & io = pool.io();
		pool.init(3).start();

		enum {
			THREAD3_GOT_WRITE_KEY = 1,
			THREAD4_GOT_KEY_FAIL,
			THREAD4_WAIT_FOR_KEY,
			THREAD5_WAIT_FOR_KEY,
			ALL_FINISHED_1,
			ALL_FINISHED_2,
		};

		auto errinfo = std::make_shared<async_error>();

		io.post([p, num, &io, &errinfo]() {

			LOG_INFO().printfln("T3: begin to write work");

			p->apply_write(io, "key2", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);

				g(ara::log::info).printfln("T3: I got the write key");
				num->signal_all(THREAD3_GOT_WRITE_KEY);
				if (token == nullptr) {
					errinfo->set_error("T3 should got the write key");
				}
				else {
					num->wait(THREAD5_WAIT_FOR_KEY);
					g(ara::log::info).printfln("T3: T4/T5 waiting for key, I will release the key");
					token = nullptr;
				}

			}, "thread 3 apply write");
		});

		io.post([p, num, &io, errinfo]() {

			ara::glog		g(ara::log::info);

			g.printfln("T4: begin to work wait for T3");
			if (!num->wait(THREAD3_GOT_WRITE_KEY)) {
				errinfo->set_error("T4 should wait for T3 got write key");
				return;
			}

			g.printfln("T4: T3 got the write key, now to my turn to try get the write key.");

			p->apply_write(io, "key2", ara::timer_val(0, 1), [num, &io, p, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);
				g.printfln("T4: I got the write key fail, because T3 hold it");
				if (token != nullptr) {
					errinfo->set_error("T4 should not got the write key, because T3 hold it, but now T4 got it");
					return;
				}
				else if (ec != boost::asio::error::timed_out) {
					errinfo->set_error("T4 should not got the write key and timeout. But now it's not timeout");
					return;
				}

				p->apply_read(io, "key2", ara::timer_val(10, 0), [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {
					ara::glog		g(ara::log::info);
					if (token == nullptr)
						errinfo->set_error(ara::printf<std::string>("T4 should got the read key, because T3 release it, but now T4 can not got it:%v",ec.message()));
					else
						g(ara::log::info).printfln("T4: I got the read key now");
					num->signal_all(ALL_FINISHED_1);

				}, "thread 2 apply 2");

				g.printfln("T4: I tell T3 I am waiting.");
				num->signal_all(THREAD4_WAIT_FOR_KEY);

			}, "thread 4 apply 1");
		});

		io.post([p, num, &io, errinfo]() {

			ara::glog		g(ara::log::info);

			g.printfln("T5: begin to work wait for T3");
			if (!num->wait(THREAD3_GOT_WRITE_KEY)) {
				errinfo->set_error("T5 should wait for T3 got write key");
				return;
			}

			g.printfln("T5: T3 got the write key, now to my turn to try get the read key.");

			p->apply_read(io, "key2", ara::timer_val(0, 1), [num, &io, p, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);
				g.printfln("T5: I got the read key fail, because T3 hold it");
				if (token != nullptr) {
					errinfo->set_error("T5 should not got the read key, because T3 hold it, but now T5 got it");
					return;
				}
				else if (ec != boost::asio::error::timed_out) {
					errinfo->set_error("T5 should not got the read key and timeout. But now it's not timeout");
					return;
				}

				p->apply_read(io, "key2", ara::timer_val(10, 0), [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {
					ara::glog		g(ara::log::info);
					if (token == nullptr)
						errinfo->set_error(ara::printf<std::string>("T5 should got the read key, because T3 release it, but now T5 can not got it:%v",ec.message()));
					else
						g(ara::log::info).printfln("T5: I got the read key now");

					num->wait(ALL_FINISHED_1);
					num->signal_all(ALL_FINISHED_2);

				}, "thread 2 apply 2");

				num->wait(THREAD4_WAIT_FOR_KEY);
				g.printfln("T5: I tell T3 I am waiting.");
				num->signal_all(THREAD5_WAIT_FOR_KEY);

			}, "thread 4 apply 1");
		});

		num->wait(ALL_FINISHED_2);
		pool.stop();

		REQUIRE(errinfo->get_error() == "");
	}
}