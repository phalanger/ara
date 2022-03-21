

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_NO_LIB

#include "3rd/Catch2/catch.hpp"

#include "ara/async_session.h"
#include "ara/async_threadpool.h"
#include "ara/event.h"
#include "test_async_helper.h"

class data
{
public:
	int		age_ = 0;
	std::string	name_;
};


TEST_CASE("async session", "[async]") {

	ara::log::init_defaul_log();
	auto oldLevel = ara::log::get_logger().set_level(ara::log::warning);
	ara::defer	_auGuard([oldLevel]() {
		ara::log::get_logger().set_level(oldLevel);
	});

	ara::async_thread_pool	pool("test");
	auto & io = pool.io();
	pool.init(4).start();

	using	AsyncSession = ara::async_session<std::string, data>;
	auto 	asyncSession = AsyncSession::make_session(1000, 101);

	enum {
		BEGIN = 1,
		TEST_1,
		TEST_2,
		FINISHED
	};

	auto num = std::make_shared<ara::event<int>>(0);
	auto errinfo = std::make_shared<async_error>();

	io.post( [asyncSession, num, errinfo, &io]() {
		num->wait(BEGIN);
		asyncSession->async_read(io, "cyt", 10_sec, [num, errinfo](const boost::system::error_code & ec, const AsyncSession::async_read_data & data) {
			if (ec)
				errinfo->set_error("Get Read 1 fail");
			else if (!data.empty())
				errinfo->set_error("Why has data on test 1");
			num->signal_all(TEST_1);
		}, "read user data");
	});

	io.post([asyncSession, num, errinfo, &io]() {
		num->wait(TEST_1);
		auto res = asyncSession->async_write_or_create(io, "cyt", 10_sec, "create user data");
		res.then([num, errinfo](const boost::system::error_code & ec, const AsyncSession::async_write_data & data) {
			if (ec)
				errinfo->set_error("Get Write/Create fail");
			else if (data.empty())
				errinfo->set_error("Why can not create data on test 2");
			else if (data->age_ != 0)
				errinfo->set_error("Why Data is not empty on test 2");
			else
				data->age_ = 101;
			num->signal_all(TEST_2);
		});
	});

	io.post([asyncSession, num, errinfo, &io]() {
		num->wait(TEST_2);
		auto res = asyncSession->async_write_or_create(io, "cyt", 10_sec, "get write user data");
		res.then([num, errinfo](const boost::system::error_code & ec, const AsyncSession::async_write_data & data) {
			if (ec)
				errinfo->set_error("Get Write fail");
			else if (data.empty())
				errinfo->set_error("Why data not exist in test 3");
			else if (data->age_ != 101)
				errinfo->set_error("Why is not 101 in test 3");
			num->signal_all(FINISHED);
		});
	});

	num->signal_all(BEGIN);
	num->wait(FINISHED);

	pool.stop();

	REQUIRE(errinfo->get_error() == "");
}
