

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch2/catch.hpp"

#include "ara/async_httpserver.h"
#include "ara/async_httpclient.h"
#include "ara/async_threadpool.h"
#include "ara/event.h"
#include "ara/log.h"
#include "test_async_helper.h"


class MyPattern : public ara::http::server_dispatch_pattern
{
public:
	virtual bool check_before_data(const ara::http::server_request & req) override {
		///
		return true;
	}
};
class MyHandle : public ara::http::server_handler
{
public:
	virtual void handle(ara::http::request_ptr req, ara::http::respond_ptr res) override {
		//Do something
	}
};

TEST_CASE("async http server", "[async]") {
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
		auto svr = ara::http::async_server::make(io, ara::http::server_options());
		std::string strContent;

		try {
			svr->add("/", [](ara::http::request_ptr req, ara::http::respond_ptr res) {
				if (req->get_abs_url() == "/") {
					res->set_code(200, "OK").add_header("Content-type","text/html").write_full_data("<body>helloworld</body>");
				}

			})
				.add(std::make_shared<MyPattern>(), std::make_shared<MyHandle>())
				.add_port(8090)
				.start();

			
			auto client = ara::http::async_client::make(io, ssl_context);
		

			auto c = client->request(ara::http::request::make("http://127.0.0.1:8090/"),
				ara::http::respond::make_simple([num, &strContent](int nCode, const std::string & strMsg, ara::http::header && h, std::string && strBody) {
					if (nCode > 0)
						strContent = std::move(strBody);
					num->signal_all(1);
				})
			);

		}
		catch (std::exception & e) {
			REQUIRE( std::string(e.what()).empty() == true );
		}

		num->wait(1);

		REQUIRE(strContent == "<body>helloworld</body>");

		svr->stop();
		pool.stop();
	}
}
