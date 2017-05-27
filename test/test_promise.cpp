
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/promise.h"
#include "ara/threadext.h"

#include <iostream>

ara::async_result<int, bool, std::string>	myfunc(int a)
{
	ara::async_result<int, bool, std::string>	result;

	ara::make_thread( [result, a]() {
	
		int b = a + 123;
		bool c = ((b % 2) == 0);

		result.set( b, c, "Hello" );
	}).detach();

	return result;
}


TEST_CASE("promise", "[base]") {

	SECTION("base") {

		auto result = myfunc(10);

		auto res2 = result.then([](int a, bool b, const std::string & s)-> ara::async_result<size_t> {
			ara::async_result<size_t> res;
			return res.set( (a - 10) * (b ? 1 : 2) + s.length());
		}).then([](size_t a) -> int {
			return static_cast<int>(a * 2);
		});

		//  myfunc : a = 10
		//		b = 133, c = false, s = "Hello"
		//	then1 : a = 133, b = false, s = "Hello"
		//		res = (133 - 10) * 2 + 5 = 123 *2 + 5 = 251
		//	then2 : a = 251
		//		res = 251 * 2
		//	so result = 502

		int a = res2.get();
		REQUIRE(a == 502);
	}
}