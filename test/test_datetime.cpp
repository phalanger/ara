
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/datetime.h"

TEST_CASE("datetime", "[base]") {

	SECTION("datetime base") {

		ara::date_time	t = ara::date_time::get_current();
		std::string str = t.format();
		ara::date_time t2(str);

		REQUIRE(t == t2);

		auto t3 = "2016-02-21 11:23:45"_date;
		REQUIRE(t3.day_of_week() == 0);
		t3.step(0, 0, 9);
		REQUIRE(t3.format("%Y%m%d") == "20160301");
	}


	SECTION("timer_val base") {

		ara::timer_val t1 = ara::timer_val::current_time();
		ara::timer_val t2 = t1;
		t1 += ara::timer_val(3, 1000);
		ara::timer_val t3 = t1 - ara::timer_val(3, 1000);

		REQUIRE(t2 == t3);

		const auto ms1 = 1000_ms;
		const auto ms2 = 1_sec;
		auto ms3 = 1_sec + 20_ms;

		REQUIRE(ms1 == ms2);
		REQUIRE(ms3.sec() == 1);
		REQUIRE(ms3.milli_sec() == 20);

		const auto t4 = 1_sec + 2_ms + 3_us + 4_ns;
		std::stringstream	sout;
		sout << t4;
		REQUIRE(sout.str() == "1s2ms3us4ns");
	}
}
