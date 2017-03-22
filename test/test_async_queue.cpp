

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch/single_include/catch.hpp"

//#include <atomic>

#include "ara/async_queue.h"
#include "ara/event.h"
#include "ara/threadext.h"
#include "ara/log.h"

class async_error
{
public:
	void	set_error(std::string && s) {
		std::lock_guard<std::mutex>		_guard(lock_);
		info_list_.emplace_back(std::move(s));
	}
	std::string get_error() const {
		std::string		str;
		for (auto & it : info_list_) {
			str += it;
			str += "\r\n";
		}
		return str;
	}
protected:
	std::mutex	lock_;
	std::list<std::string>	info_list_;
};

TEST_CASE("async queue", "[async]" ){

	SECTION("base") {

		ara::log::init_defaul_log();
		auto oldLevel = ara::log::get_logger().set_level(ara::log::warning);
		ara::defer	_auGuard([oldLevel]() {
			ara::log::get_logger().set_level(oldLevel);
		});

		typedef ara::async_queue<std::string>	name_queue;

		auto p = name_queue::make_queue(10);
		auto num = std::make_shared<ara::event<int>>(0);

		boost::asio::io_service		io;

		enum {
			THREAD1_GOT_KEY = 1,
			THREAD2_GOT_KEY_FAIL,
			THREAD1_RELEASE_KEY,
			ALL_FINISHED,
		};

		boost::asio::io_service::work	worker(io);
		auto errinfo = std::make_shared<async_error>();

		io.post([p, num, &io, errinfo]() {

			LOG_INFO().printfln("T1: begin to work");

			p->apply(io, "key1", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);

				g(ara::log::info).printfln("T1: I got the key");
				num->signal_all(THREAD1_GOT_KEY);
				if (token == nullptr)
					errinfo->set_error("Token is null while T1 apply");
				else if (!num->wait(THREAD2_GOT_KEY_FAIL) )
					errinfo->set_error("T1 fail to wait for thread2 got key fail");
				else {
					g(ara::log::info).printfln("T1: T2 get key fail, I will release the key");
					token = nullptr;
					num->signal_all(THREAD1_RELEASE_KEY);
				}

			}, "thread 1 apply 1");
		});

		io.post([p, num, &io, errinfo]() {

			ara::glog		g(ara::log::info);

			g.printfln("T2: begin to work wait for T1");
			REQUIRE(num->wait(THREAD1_GOT_KEY));
			g.printfln("T2: T1 got the key, now to my turn to try get the key.");
			
			p->apply(io, "key1", ara::timer_val(0, 1), [num, &io, p, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);
				g.printfln("T2: I got the key fail, because T1 hold it");
				if (token != nullptr) {
					errinfo->set_error("T2 should not got the token because T1 hold it. But now T2 got it.");
				} else if (ec != boost::asio::error::timed_out) {
					errinfo->set_error("T2 should not got the token and should be timeout. But now T2 is not timeout.");
				} else {
					g.printfln("T2: I tell T1 to release the key");
					num->signal_all(THREAD2_GOT_KEY_FAIL);

					num->wait(THREAD1_RELEASE_KEY);
					g(ara::log::info).printfln("T2: T1 has release key, I will try to get the key again");

					io.post([p, num, &io, errinfo]() {
						p->apply(io, "key1", ara::timer_val(0, 0), [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {
							ara::glog		g(ara::log::info);
							if (token == nullptr) {
								errinfo->set_error("T2 should got token after T1 release it. But T2 can not got it now.");
							} else {
								g(ara::log::info).printfln("T2: I got the key now");
								num->signal_all(ALL_FINISHED);
							}

						}, "thread 2 apply 2");
					});

				}

			}, "thread 2 apply 1");
		});

		auto t1 = ara::make_thread([&io]() {
			io.run();
		});
		auto t2 = ara::make_thread([&io]() {
			io.run();
		});

		num->wait(ALL_FINISHED);
		io.stop();

		t1.join();
		t2.join();

		REQUIRE(errinfo->get_error() == "");
	}

	SECTION("base 2") {

		ara::log::init_defaul_log();
		auto oldLevel = ara::log::get_logger().set_level(ara::log::warning);
		ara::defer	_auGuard([oldLevel]() {
			ara::log::get_logger().set_level(oldLevel);
		});

		typedef ara::async_queue<std::string>	name_queue;

		auto p = name_queue::make_queue(10);
		auto num = std::make_shared<ara::event<int>>(0);

		boost::asio::io_service		io;

		enum {
			THREAD3_GOT_KEY = 1,
			THREAD4_GOT_KEY_FAIL,
			THREAD4_WAIT_FOR_KEY,
			ALL_FINISHED,
		};

		boost::asio::io_service::work	worker(io);
		auto errinfo = std::make_shared<async_error>();

		io.post([p, num, &io, &errinfo]() {

			LOG_INFO().printfln("T3: begin to work");

			p->apply(io, "key2", ara::timer_val::max_time, [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);

				g(ara::log::info).printfln("T3: I got the key");
				num->signal_all(THREAD3_GOT_KEY);
				if (token == nullptr) {
					errinfo->set_error("T3 should got the key");
				} else {
					num->wait(THREAD4_WAIT_FOR_KEY);
					g(ara::log::info).printfln("T3: T4 waiting for key, I will release the key");
					token = nullptr;
				}

			}, "thread 3 apply 1");
		});

		io.post([p, num, &io, errinfo]() {

			ara::glog		g(ara::log::info);

			g.printfln("T4: begin to work wait for T3");
			if (!num->wait(THREAD3_GOT_KEY)) {
				errinfo->set_error("T4 should wait for T3 got key");
				return;
			}

			g.printfln("T4: T3 got the key, now to my turn to try get the key.");

			p->apply(io, "key2", ara::timer_val(0, 1), [num, &io, p, errinfo](const boost::system::error_code & ec, ara::async_token token) {

				ara::glog		g(ara::log::info);
				g.printfln("T4: I got the key fail, because T3 hold it");
				if (token != nullptr) {
					errinfo->set_error("T4 should not got the token, because T3 hold it, but now T4 got it");
					return;
				} else if (ec != boost::asio::error::timed_out) {
					errinfo->set_error("T4 should not got the token and timeout. But now it's not timeout");
					return;
				}

				p->apply(io, "key2", ara::timer_val(10, 0), [num, errinfo](const boost::system::error_code & ec, ara::async_token token) {
					ara::glog		g(ara::log::info);
					if (token == nullptr) {
						errinfo->set_error("T4 should got the token, because T3 release it, but now T4 can not got it");
						return;
					}
					g(ara::log::info).printfln("T4: I got the key now");
					num->signal_all(ALL_FINISHED);

				}, "thread 2 apply 2");

				g.printfln("T4: I tell T3 I am waiting.");
				num->signal_all(THREAD4_WAIT_FOR_KEY);

			}, "thread 2 apply 1");
		});

		auto t1 = ara::make_thread([&io]() {
			io.run();
		});
		auto t2 = ara::make_thread([&io]() {
			io.run();
		});

		num->wait(ALL_FINISHED);
		io.stop();

		t1.join();
		t2.join();

		REQUIRE(errinfo->get_error() == "");
	}
}