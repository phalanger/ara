

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch/single_include/catch.hpp"

#include "ara/async_httpserver.h"
#include "ara/async_threadpool.h"
#include "ara/event.h"
#include "ara/log.h"
#include "test_async_helper.h"

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


		ara::http::async_server svr(io, ssl_context);
		svr.add_dispatch_data("/*", [](ara::http::async_request_ptr req, ara::http::async_respond_ptr res) {
		
			if (req->get_url() == "/") {
				res->set_code(200, "OK").add_header("Content-type","text/html").write_full_data("<body>helloworld</body>");
			}

		});


		num->wait(1);
		pool.stop();
	}
}
