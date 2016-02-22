
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
#include "ara/flags.h"

BOOST_AUTO_TEST_SUITE(flags)

enum {
	TESTFLAG_1 = 0x01,
	TESTFLAG_2 = 0x02,
	TESTFLAG_3 = 0x04,
	TESTFLAG_4 = 0x08,
};

BOOST_AUTO_TEST_CASE(flags_base)
{
	ara::flags<int>		flag1({ TESTFLAG_1 , TESTFLAG_2, TESTFLAG_4 });
	
	BOOST_REQUIRE_EQUAL(flag1.check(TESTFLAG_1), true);
	BOOST_REQUIRE_EQUAL(flag1.check(TESTFLAG_3), false);

	BOOST_REQUIRE_EQUAL(flag1.check_all(TESTFLAG_1 | TESTFLAG_2), true);
	BOOST_REQUIRE_EQUAL(flag1.check_all(TESTFLAG_1 | TESTFLAG_3), false);

	BOOST_REQUIRE_EQUAL(flag1.check_marks(TESTFLAG_1 | TESTFLAG_3, TESTFLAG_1), true);
	BOOST_REQUIRE_EQUAL(flag1.check_marks(TESTFLAG_1 | TESTFLAG_3, TESTFLAG_3), false);

	flag1 |= TESTFLAG_3;
	BOOST_REQUIRE_EQUAL(flag1.check_marks(TESTFLAG_1 | TESTFLAG_3, (TESTFLAG_1 | TESTFLAG_3)), true);
}

BOOST_AUTO_TEST_SUITE_END()
