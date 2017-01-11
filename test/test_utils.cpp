
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/utils.h"

#include <vector>

TEST_CASE("utils", "[base]") {

	std::vector<int>	ary = { 1, 2, 3, 4, 5, 6 };
	size_t i = ary.size();
	for (int n : ara::reverse_range(ary)) {
		REQUIRE(n == ary[--i]);
	}

}
