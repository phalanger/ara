
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/json.h"

TEST_CASE("json", "[base]") {

	SECTION("base") {

		auto doc = ara::jsvar::init();
		ara::jsvar		var(doc);

		REQUIRE(var.is_null());

	}

}