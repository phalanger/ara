
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/stringext.h"

TEST_CASE("stringext", "[base]") {

	SECTION("constructor") {
		std::string str1 = " hello\r\t";
		const std::string str2 = "\rhello world\n\t ";

		size_t i = 0;
		for (auto ch : ara::strext(str1)) {
			REQUIRE(ch == str1[i++]);
		}
		i = 0;
		for (auto ch : ara::strext(str2)) {
			REQUIRE(ch == str2[i++]);
		}
	}

	SECTION("trim") {
		std::string str1 = " hello\r\t";
		const std::string str2 = "\rhello world\n\t ";

		auto re1 = ara::strext(str1).trim_left(" \t\r\n");
		REQUIRE(re1 == "hello\r\t");

		auto re2 = ara::strext(str2).trim_right(" \t\r\n");
		REQUIRE(re2 == "\rhello world");

		auto re3 = ara::strext(str2).trim(" \t\r\n");
		REQUIRE(re3 == "hello world");
	}

	SECTION("trim_inplace") {
		const std::string str1 = "\rhello world\n\t ";

		std::string re1 = str1;
		ara::strext(re1).trim_left_inplace(" \t\r\n");
		REQUIRE(re1 == "hello world\n\t ");

		re1 = str1;
		ara::strext(re1).trim_right_inplace(" \t\r\n");
		REQUIRE(re1 == "\rhello world");

		re1 = str1;
		ara::strext(re1).trim_inplace(" \t\r\n");
		REQUIRE(re1 == "hello world");
	}

	SECTION("to int") {
		const std::string str1 = "";
		REQUIRE(ara::strext(str1).to_int<int>() == int(0));

		const std::string str2 = "123";
		REQUIRE(ara::strext(str2).to_int<int>() == int(123));
		REQUIRE(ara::strext(str2).to<int>() == int(123));

		const std::string str3 = "a123";
		int n = ara::strext(str3).to_int<int, 16>();
		REQUIRE(n == int(0xa123));

		const std::string str4 = "a123";
		int n2 = ara::strext(str4).to_int<int>();
		REQUIRE(n2 == int(0));

		int n3 = ara::strext(ara::ref_string("112345")).to_int<int, 8>();
		REQUIRE(n3 == int(0112345));

		int n4 = ara::strext(ara::ref_string("112345")).to<int>();
		REQUIRE(n4 == int(112345));
	}

	SECTION("find int") {
		const std::string str1 = " wer\r\n123fqw";
		REQUIRE(ara::strext(str1).find_int<int>() == int(123));
		int a = ara::strext(str1).find_int<int, 16>();
		REQUIRE(a == int(0x0e));
		a = ara::strext(str1).find_int<int, 16>(3);
		REQUIRE(a == int(0x123f));
		a = ara::strext(str1).find_int<int, 8>(3);
		REQUIRE(a == int(0123));
	}

	SECTION("from int") {
		std::string str1;
		ara::strext(str1).append_int(100);
		REQUIRE(str1 == "100");

		std::string str2;
		ara::strext(str2).append_int<int, 16>(12);
		REQUIRE(str2 == "C");

		std::string str3;
		ara::strext(str3).append_int<int, 16, true>(12);
		REQUIRE(str3 == "c");
	}

	SECTION("string type") {
		REQUIRE(ara::is_string<std::string>::value);
		REQUIRE(ara::is_string<std::wstring>::value);
		REQUIRE(ara::is_string<std::u16string>::value);
		REQUIRE(ara::is_string<std::u32string>::value);
		REQUIRE(ara::is_string<ara::ref_string>::value);
		REQUIRE_FALSE(ara::is_string<int>::value);
	}

	SECTION("append string") {

		std::string	strRes;
		auto s1 = ara::strext(strRes);

		s1 += "Hello ";
		s1 += L"world";

		REQUIRE(strRes == "Hello world");


		std::wstring	strRes2;
		auto s2 = ara::strext(strRes2);

		s2 += "Hello ";
		s2 += L"world";
		s2 += s1.str();

		REQUIRE(ara::strext(strRes2).to<std::string>() == "Hello worldHello world");

		std::string		strRes3 = "abcdefgh";
		REQUIRE(ara::strext(strRes3).to<std::string>() == "abcdefgh");
	}

	SECTION("printf string") {
		std::string	strRes;
		ara::strext(strRes).printf("Hello %d ha%s%shaha", 100, L"ha", "ha");

		REQUIRE(strRes == "Hello 100 hahahahaha");

		REQUIRE(ara::str_printf<std::string>(L"Hello %02d", 8) == "Hello 08");

		size_t n = 100;
		int m = -10;
		REQUIRE(ara::str_printf<std::string>(L"Test %v, %v", n, m) == "Test 100, -10");
	}

	SECTION("printf stream") {
		std::stringstream	s;
		ara::stream_printf(s).printf("Hello world %d", 100);

		REQUIRE(s.str() == "Hello world 100");

		std::stringstream	s2;
		ara::stream_printf(s2, "And say hello again, %d", 102);

		REQUIRE(s2.str() == "And say hello again, 102");
	}

	SECTION("ara::printf") {
		auto s = ara::printf<std::string>("%d + %d = %d", 1, 2, 3);
		REQUIRE(s == "1 + 2 = 3");

		std::stringstream s2;
		ara::printf(s2, "%d - %d = %d", 3, 2, 1);
		REQUIRE(s2.str() == "3 - 2 = 1");
	}

	SECTION("ara::snprintf") {
		char buf[16];

		size_t n = ara::snprintf(buf, 16, "Hello %s", L"world");
		REQUIRE(n == 11);
		REQUIRE(std::string(buf) == "Hello world");

		n = ara::snprintf(buf, 16, "Hello %s %d", L"world", 100000);
		REQUIRE(n == 16);
		REQUIRE(std::string(buf, n) == "Hello world 1000");

		n = ara::snprintf(buf, 16, "Hello %s %05d", "world", 123);
		REQUIRE(n == 16);
		REQUIRE(std::string(buf, n) == "Hello world 0012");
	}

	SECTION("nocase_compare") {
		ara::nocase_string_compare<std::string>		cmp;
		REQUIRE_FALSE(cmp("Hello", "heLLO"));
		REQUIRE_FALSE(cmp("Hello1", "heLLO"));
		REQUIRE(cmp("Hello", "heLLO1"));

		REQUIRE(cmp("Hella", "heLLO"));
		REQUIRE_FALSE(cmp("Hellp", "heLLO"));
		REQUIRE(cmp("Hella", "heLLP"));

		std::map<std::string, int, ara::nocase_string_compare<std::string>>	mapData;
		mapData["AA"] = 1;
		REQUIRE(mapData["aa"] == 1);

		REQUIRE(ara::nocase_string_compare<std::string>::compare("Hello", "heLLO") == 0);
		REQUIRE(ara::nocase_string_compare<std::string>::compare("Hello1", "heLLO") == 1);
		REQUIRE(ara::nocase_string_compare<std::string>::compare("Hello", "heLLO1") == -1);

		REQUIRE(ara::nocase_string_compare<std::string>::compare("Hella", "heLLO") == -1);
		REQUIRE(ara::nocase_string_compare<std::string>::compare("Hellp", "heLLO") == 1);
		REQUIRE(ara::nocase_string_compare<std::string>::compare("Hella", "heLLP") == -1);
	}
}
