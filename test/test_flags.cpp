
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/flags.h"

enum {
	TESTFLAG_1 = 0x01,
	TESTFLAG_2 = 0x02,
	TESTFLAG_3 = 0x04,
	TESTFLAG_4 = 0x08,
	TESTFLAG_5 = 0x10,
};

TEST_CASE("flags", "[base]") {

	SECTION("base") {

		ara::flags<int>		flag1({ TESTFLAG_1 , TESTFLAG_2, TESTFLAG_4 });

		REQUIRE(flag1.check(TESTFLAG_1) == true);
		REQUIRE(flag1.check(TESTFLAG_3) == false);

		REQUIRE(flag1.check_all(TESTFLAG_1 | TESTFLAG_2));
		REQUIRE_FALSE(flag1.check_all(TESTFLAG_1 | TESTFLAG_3));

		REQUIRE(flag1.check_one(TESTFLAG_1 | TESTFLAG_3));
		REQUIRE_FALSE(flag1.check_one(TESTFLAG_3 | TESTFLAG_5));

		REQUIRE(flag1.check_marks(TESTFLAG_1 | TESTFLAG_3, TESTFLAG_1));
		REQUIRE_FALSE(flag1.check_marks(TESTFLAG_1 | TESTFLAG_3, TESTFLAG_3));

		flag1 |= TESTFLAG_3;
		REQUIRE(flag1.check_marks(TESTFLAG_1 | TESTFLAG_3, (TESTFLAG_1 | TESTFLAG_3)));

		flag1.clear_flags(TESTFLAG_1, TESTFLAG_3);
		REQUIRE(flag1.check_all(TESTFLAG_2 , TESTFLAG_4));
		REQUIRE_FALSE(flag1.check_all(TESTFLAG_2, TESTFLAG_3, TESTFLAG_4));
		REQUIRE(flag1.check_one(TESTFLAG_2, TESTFLAG_3, TESTFLAG_4));
		REQUIRE_FALSE(flag1.check_one(TESTFLAG_1, TESTFLAG_3, TESTFLAG_5));

	}

}
