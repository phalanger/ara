
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/datetime.h"

class DateTimeWithMSTraits
{
public:
	static inline void	set_time(uint64_t & tar, time_t src) {
		tar = (src * 1000) | (tar % 1000);
	}
	static inline time_t get_time(uint64_t src) {
		return src / 1000;
	}
};

class date_time_ms : public ara::internal::date_time_imp<uint64_t, DateTimeWithMSTraits>
{
public:
	typedef ara::internal::date_time_imp<uint64_t, DateTimeWithMSTraits>	parent;
	template<typename...args>
	date_time_ms(args... a) : parent(a...) {}

	int		get_ms() const {
		return get_raw() % 1000;
	}

	void	set_ms(int n) {
		uint64_t & c = get_raw();
		c = (c / 1000) * 1000 + (n % 1000);
	}
};

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


		ara::date_time	that_day(2011, 3, 28, 10, 30, 00);
		int n = that_day.day_of_year();
		REQUIRE(n == 86);

		ara::date_time	day2(time(NULL));
		REQUIRE(day2.day_of_year() == t.day_of_year());


		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		ara::date_time	day3(now);
		REQUIRE(day3.day_of_year() == t.day_of_year());

		ara::date_time	day4(std::chrono::system_clock::now());
		REQUIRE(day4.day_of_year() == t.day_of_year());

		ara::date_time	day5("2016 02 21 11 23 45", "%Y %m %d %H %M %S");
		day5.step(0, 0, 0, 9 * 24);
		REQUIRE(day5 == t3);
	}

	SECTION("datetime with ms") {
		date_time_ms	t2(2011, 3, 28, 10, 30, 00);
		t2.set_ms(123);
		REQUIRE(t2.get_ms() == 123);
		REQUIRE(t2.day_of_year() == 86);
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
