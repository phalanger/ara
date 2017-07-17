

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch/single_include/catch.hpp"

#include "ara/async_httpclient.h"
#include "ara/async_threadpool.h"
#include "ara/event.h"
#include "ara/log.h"
#include "test_async_helper.h"

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }

TEST_CASE("async http client", "[async]") {
	SECTION("base") {

		ara::log::init_defaul_log();
		auto oldLevel = ara::log::get_logger().set_level(ara::log::warning);
		ara::defer	_auGuard([oldLevel]() {
			ara::log::get_logger().set_level(oldLevel);
		});

		ara::async_thread_pool	pool("test");
		auto & io = pool.io();
		pool.init(2).start();

		auto num = std::make_shared<ara::event<int>>(0);

		boost::asio::ssl::context	ssl_context(boost::asio::ssl::context::sslv23_client);
		auto client = ara::http::async_client::make(io, ssl_context);

		auto c = client->request(ara::http::request::make("https://163.com"),
			ara::http::respond::make_simple([num](int nCode, const std::string & strMsg, ara::http::header && h, std::string && strBody) {
			if (nCode > 0)
				std::cout << "Body:" << strBody << std::endl;
			num->signal_all(1);
		})
		);

		num->wait(1);
		pool.stop();
	}
}
