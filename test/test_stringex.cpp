
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test_suite.hpp>
//#include <boost/test/unit_test.hpp>
#include "ara/stringext.h"

BOOST_AUTO_TEST_SUITE(stringext)

BOOST_AUTO_TEST_CASE(strext_constructor)
{
	std::string str1 = " hello\r\t";
	const std::string str2 = "\rhello world\n\t ";

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

BOOST_AUTO_TEST_CASE(strext_to_int)
{
	const std::string str1 = "";
	BOOST_REQUIRE_EQUAL( ara::strext(str1).to_int<int>(), int(0));

	const std::string str2 = "123";
	BOOST_REQUIRE_EQUAL(ara::strext(str2).to_int<int>(), int(123));
	BOOST_REQUIRE_EQUAL(ara::strext(str2).to<int>(), int(123));

	const std::string str3 = "a123";
	int n = ara::strext(str3).to_int<int, 16>();
	BOOST_REQUIRE_EQUAL(n, int(0xa123));

	const std::string str4 = "a123";
	int n2 = ara::strext(str4).to_int<int>();
	BOOST_REQUIRE_EQUAL(n2, int(0));

	int n3 = ara::strext(ara::ref_string("112345")).to_int<int,8>();
	BOOST_REQUIRE_EQUAL(n3, int(0112345));

	int n4 = ara::strext(ara::ref_string("112345")).to<int>();
	BOOST_REQUIRE_EQUAL(n4, int(112345));
}

BOOST_AUTO_TEST_CASE(strext_from_int)
{
	std::string str1;
	ara::strext(str1).append_int(100);
	BOOST_REQUIRE_EQUAL(str1, "100");

	std::string str2;
	ara::strext(str2).append_int<int, 16>(12);
	BOOST_REQUIRE_EQUAL(str2, "C");

	std::string str3;
	ara::strext(str3).append_int<int, 16,true>(12);
	BOOST_REQUIRE_EQUAL(str3, "c");
}

BOOST_AUTO_TEST_CASE(detech_string_type)
{
	BOOST_REQUIRE_EQUAL(ara::is_string<std::string>::value, true);
	BOOST_REQUIRE_EQUAL(ara::is_string<ara::ref_string>::value, true);
	BOOST_REQUIRE_EQUAL(ara::is_string<int>::value, false);
}

BOOST_AUTO_TEST_CASE(append_string)
{
	std::string	strRes;
	auto s1 = ara::strext(strRes);

	s1 += "Hello ";
	s1 += L"world";

	BOOST_REQUIRE_EQUAL(strRes, "Hello world");


	std::wstring	strRes2;
	auto s2 = ara::strext(strRes2);

	s2 += "Hello ";
	s2 += L"world";
	s2 += s1.str();

	BOOST_REQUIRE_EQUAL( ara::strext(strRes2).to<std::string>(), "Hello worldHello world");

	std::string		strRes3 = "abcdefgh";
	BOOST_REQUIRE_EQUAL(ara::strext(strRes3).to<std::string>(), "abcdefgh");
}

BOOST_AUTO_TEST_CASE(printf_string)
{
	std::string	strRes;
	ara::strext(strRes).printf("Hello %d hahaha", 100);

	BOOST_REQUIRE_EQUAL(strRes, "Hello 100 hahaha");

	BOOST_REQUIRE_EQUAL(ara::str_printf<std::string>(L"Hello %02d",8), "Hello 08" );
}

BOOST_AUTO_TEST_SUITE_END()
