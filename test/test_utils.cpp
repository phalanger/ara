
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/utils.h"

#include <vector>

TEST_CASE("utils", "[base]") {

	SECTION("reverse_range") {
		std::vector<int>	ary = { 1, 2, 3, 4, 5, 6 };
		size_t i = ary.size();
		for (int n : ara::reverse_range(ary)) {
			REQUIRE(n == ary[--i]);
		}
	}

	SECTION("defer") {
		int n = 10;

		{
			ara::defer	_a([&n]() { ++n; });

			++n;
			REQUIRE(n == 11);
		}
		REQUIRE(n == 12);


		{
			ARA_DEFER(
				++n; 
				n += 2; 
			)
			++n;
			REQUIRE(n == 13);
		}
		REQUIRE(n == 16);
	}


}
