
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/fixed_string.h"

TEST_CASE("fixed_string", "[base]") {

	SECTION("base") {

		char buf[16];
		ara::fixed_string		str1(buf, 16);

		str1 = "hello world";
		REQUIRE(str1.size() == 11);

		str1 += " hahaha";
		REQUIRE(str1.size() == 16);

		REQUIRE(str1 == "hello world haha");

		str1.insert(2, "345");
		REQUIRE(str1 == "he345llo world h");
	}

	SECTION("base wchar_t") {

		wchar_t buf[16];
		ara::fixed_wstring		str1( buf, 16);

		str1 = L"hello world";
		REQUIRE(str1.size() == 11);

		str1 += L" hahaha";
		REQUIRE(str1.size() == 16);

		REQUIRE(str1 == L"hello world haha");
		str1.insert(2, L"345");
		REQUIRE(str1 == L"he345llo world h");
	}
}
