
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/key_string.h"

TEST_CASE("key_string", "[base]") {

	REQUIRE(sizeof(ara::key_string) == sizeof(void *) * 2);

	SECTION("base") {

		ara::key_string		str1("hello world");
		REQUIRE(str1.size() == 11);
		REQUIRE(str1.data_type() == ara::key_string::TYPE_CONST);

		str1 = "haha";
		REQUIRE(str1.size() == 4);
		REQUIRE(str1.data_type() == ara::key_string::TYPE_BUF);

		str1 = ara::key_string::copy("aaa");
		REQUIRE(str1.size() == 3);
		REQUIRE(str1.data_type() == ara::key_string::TYPE_BUF);

		str1 = ara::key_string::copy("aaa123456");
		REQUIRE(str1.size() == 9);
		REQUIRE(str1.data_type() == ara::key_string::TYPE_STORE);

		ara::key_string str2 = str1;
		REQUIRE(str2.size() == 9);
		REQUIRE(str2 == "aaa123456");
		REQUIRE(str2.ref_count() == 2);
	}

	SECTION("base wchar_t") {
		ara::key_wstring		str1(L"hello world");
		REQUIRE(str1.size() == 11);
		REQUIRE(str1.data_type() == ara::key_wstring::TYPE_CONST);

		str1 = L"ha";
		REQUIRE(str1.size() == 2);
		REQUIRE(str1.data_type() == ara::key_wstring::TYPE_BUF);

		str1 = ara::key_wstring::copy(L"aa");
		REQUIRE(str1.size() == 2);
		REQUIRE(str1.data_type() == ara::key_wstring::TYPE_BUF);

		str1 = ara::key_wstring::copy(L"aaaa");
		REQUIRE(str1.size() == 4);
		REQUIRE(str1.data_type() ==  (sizeof(wchar_t) == 2 ? ara::key_wstring::TYPE_BUF : ara::key_wstring::TYPE_STORE));

		str1 = ara::key_wstring::copy(L"aaa123456");
		REQUIRE(str1.size() == 9);
		REQUIRE(str1.data_type() == ara::key_wstring::TYPE_STORE);

		ara::key_wstring str2 = str1;
		REQUIRE(str2.size() == 9);
		REQUIRE(str2 == L"aaa123456");
		REQUIRE(str2.ref_count() == 2);
	}
}
