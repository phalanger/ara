

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch/single_include/catch.hpp"

#include "ara/async_httpclient.h"
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

		boost::asio::ssl::context	ssl_context(boost::asio::ssl::context::tlsv11_client);
		auto client = ara::http::async_client::make(io, ssl_context);

		std::string strContent;

		auto c = client->request(ara::http::request::make("https://163.com/"),
			ara::http::respond::make_simple([num, &strContent](int nCode, const std::string & strMsg, ara::http::header && h, std::string && strBody) {
			if (nCode > 0)
				strContent = std::move(strBody);
			num->signal_all(1);
		})
		);

		num->wait(1);
		pool.stop();

		REQUIRE(strContent.find("NetEase Devilfish") != std::string::npos);
	}
}
