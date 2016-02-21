
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "ara/datetime.h"

BOOST_AUTO_TEST_SUITE(datetime)

BOOST_AUTO_TEST_CASE(datetime_base)
{
	ara::date_time	t = ara::date_time::get_current();
	std::string str = t.format();
	ara::date_time t2(str);

	BOOST_REQUIRE_EQUAL(t, t2);

	auto t3 = "2016-02-21 11:23:45"_date;
	BOOST_REQUIRE_EQUAL(t3.day_of_week(), 0);
	t3.step(0, 0, 9);
	BOOST_REQUIRE_EQUAL(t3.format("%Y%m%d"), "20160301");
}

BOOST_AUTO_TEST_CASE(timer_val_base)
{
	ara::timer_val t1 = ara::timer_val::current_time();
	ara::timer_val t2 = t1;
	t1 += ara::timer_val(3, 1000);
	ara::timer_val t3 = t1 - ara::timer_val(3, 1000);

	BOOST_REQUIRE_EQUAL(t2, t3);

	const auto ms1 = 1000_ms;
	const auto ms2 = 1_sec;
	auto ms3 = 1_sec + 20_ms;

	BOOST_REQUIRE_EQUAL(ms1, ms2);
	BOOST_REQUIRE_EQUAL(ms3.sec(), 1);
	BOOST_REQUIRE_EQUAL(ms3.milli_sec(), 20);

	const auto t4 = 1_sec + 2_ms + 3_us + 4_ns;
	std::stringstream	sout;
	sout << t4;
	BOOST_REQUIRE_EQUAL(sout.str(), "1s2ms3us4ns");
}

BOOST_AUTO_TEST_SUITE_END()
