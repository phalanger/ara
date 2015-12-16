
#define BOOST_TEST_MODULE ara

#include "boost/test/included/unit_test.hpp"

#include "ara/stringext.h"

BOOST_AUTO_TEST_SUITE(stringext)

BOOST_AUTO_TEST_CASE(strext_constructor)
{
	std::string str1 = " hello\r\t";
	const std::string str2 = "\rhello world\n\t ";

	auto ext1 = ara::strext(str1);
	auto ext2 = ara::strext(str2);

	size_t i = 0;
	for (auto ch : ara::strext(str1)) {
		BOOST_REQUIRE_EQUAL(ch , str1[i++]);
	}
	i = 0;
	for (auto ch : ara::strext(str2)) {
		BOOST_REQUIRE_EQUAL(ch , str2[i++]);
	}
}

BOOST_AUTO_TEST_CASE(strext_trim)
{
	std::string str1 = " hello\r\t";
	const std::string str2 = "\rhello world\n\t ";

	auto re1 = ara::strext(str1).trim_left(" \t\r\n");
	BOOST_REQUIRE_EQUAL(re1, "hello\r\t");

	auto re2 = ara::strext(str2).trim_right(" \t\r\n");
	BOOST_REQUIRE_EQUAL(re2, "\rhello world");

	auto re3 = ara::strext(str2).trim(" \t\r\n");
	BOOST_REQUIRE_EQUAL(re3, "hello world");
}

BOOST_AUTO_TEST_CASE(strext_trim_inplace)
{
	const std::string str1 = "\rhello world\n\t ";

	std::string re1 = str1;
	ara::strext(re1).trim_left_inplace(" \t\r\n");
	BOOST_REQUIRE_EQUAL(re1, "hello world\n\t ");

	re1 = str1;
	ara::strext(re1).trim_right_inplace(" \t\r\n");
	BOOST_REQUIRE_EQUAL(re1, "\rhello world");

	re1 = str1;
	ara::strext(re1).trim_inplace(" \t\r\n");
	BOOST_REQUIRE_EQUAL(re1, "hello world");
}

BOOST_AUTO_TEST_SUITE_END()
