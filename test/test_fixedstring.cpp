
#include "3rd/Catch/single_include/catch.hpp"

#include "ara/fixed_string.h"
#include <string.h>

TEST_CASE("fixed_string", "[base]") {

	SECTION("base") {

		char buf[16];
		ara::fixed_string		str1(buf, 16);

		str1 = "hello world";
		REQUIRE(str1.size() == 11);

		str1 += " hahaha";
		REQUIRE(str1.size() == 16);
		REQUIRE(str1.length() == 16);

		REQUIRE(str1 == "hello world haha");

		str1.insert(2, "345");
		REQUIRE(str1 == "he345llo world h");

		ara::fixed_string	str2(buf, buf + sizeof(buf));
		REQUIRE(str2.empty());

		ara::fixed_string	str3(str1);
		REQUIRE(str1 == str3);
		REQUIRE(str1 == std::string("he345llo world h"));
		REQUIRE(str1 != std::string("he345llo world 2"));
		REQUIRE(str1 != std::string("he345llo world"));
		REQUIRE(str1 > std::string("he345llo world g"));
		REQUIRE(str1 > std::string("he345llo world"));
		REQUIRE(str1 >= std::string("he345llo world h"));
		REQUIRE(str1 >= std::string("he345llo world g"));
		REQUIRE(str1 < std::string("he345llo world i"));
		REQUIRE(str1 < std::string("he345llo world h2"));
		REQUIRE(str1 <= std::string("he345llo world h"));
		REQUIRE(str1 <= std::string("he345llo world i"));
		const char * s1 = "01234567890abcdefghijk";
		str3.assign(s1, s1 + strlen(s1));
		REQUIRE(str3 == std::string(s1, sizeof(buf)));

		char buf2[] = "H22323";
		ara::fixed_string	str6(buf2, sizeof(buf2) - 1, sizeof(buf2));
		REQUIRE(str6 == buf2);
		REQUIRE(str6 != "H223234");
		REQUIRE(str6 != "H22324");
		REQUIRE(str6 < "H22324");
		REQUIRE(str6 <= "H22324");
		REQUIRE(str6 <= "H223234");
		REQUIRE(str6 > "H22322");
		REQUIRE(str6 > "H2232");
		REQUIRE(str6 >= "H2232");
		REQUIRE(str6 >= "H22322");

		REQUIRE(str6[2] >= '2');
		REQUIRE(str6.at(3) >= '3');
		REQUIRE(str6.substr(0, 2) == "H2");
		REQUIRE(str6.substr(100, 2) == "");
		REQUIRE(str6.substr(1, 100) == "22323");

		REQUIRE(str6.substring(0, 2) == "H2");
		REQUIRE(str6.substring(100, 2) == "");
		REQUIRE(str6.substring(1, 100) == "22323");

		REQUIRE(str6.substring(-2) == "23");
		REQUIRE(str6.substring(2, -2) == "232");
		REQUIRE(str6.substring(-10, 100) == "");
		REQUIRE(str6.substring(10, -100) == "");

		REQUIRE(str6.find('2') == 1);
		REQUIRE(str6.find('3', 4) == 5);
		REQUIRE(str6.find("H2") == 0);
		REQUIRE(str6.find("23") == 2);
		REQUIRE(str6.find("23", 3) == 4);
		REQUIRE(str6.find("A", 3) == ara::fixed_string::npos);
		REQUIRE(str6.find(std::string("23"), 2) == 2);
		REQUIRE(str6.rfind('2') == 4);
		REQUIRE(str6.rfind('3', 4) == 3);
		REQUIRE(str6.rfind("H2") == 0);
		REQUIRE(str6.rfind("23") == 4);
		REQUIRE(str6.rfind("23", 3) == 2);
		REQUIRE(str6.rfind("A", 3) == ara::fixed_string::npos);
		REQUIRE(str6.rfind(std::string("23"), 3) == 2);

		REQUIRE(str6.find_first_of("BHA") == 0);
		REQUIRE(str6.find_first_of("BHA", 0) == 0);
		REQUIRE(str6.find_first_of("BAC", 0) == ara::fixed_string::npos);
		REQUIRE(str6.find_first_of("2345", 2) == 2);
		REQUIRE(str6.find_first_of(std::string("2345"), 2) == 2);
		REQUIRE(str6.find_first_not_of("abc", 0) == 0);
		REQUIRE(str6.find_first_not_of("BHA", 0) == 1);
		REQUIRE(str6.find_first_not_of("H23", 0) == ara::fixed_string::npos);
		REQUIRE(str6.find_first_not_of("H246", 2) == 3);
		REQUIRE(str6.find_first_not_of(std::string("H246"), 2) == 3);

		REQUIRE(str6.find_last_of("BHA") == 0);
		REQUIRE(str6.find_last_of("BHA", 0) == ara::fixed_string::npos);
		REQUIRE(str6.find_last_of("BAC", 0) == ara::fixed_string::npos);
		REQUIRE(str6.find_last_of("2345") == 5);
		REQUIRE(str6.find_last_of("2345", 2) == 1);
		REQUIRE(str6.find_last_of(std::string("2345"), 2) == 1);
		REQUIRE(str6.find_last_not_of("abc") == 5);
		REQUIRE(str6.find_last_not_of("BHA", 100) == 5);
		REQUIRE(str6.find_last_not_of("H23") == ara::fixed_string::npos);
		REQUIRE(str6.find_last_not_of("H246", 4) == 3);
		REQUIRE(str6.find_last_not_of(std::string("H246"), 4) == 3);

		REQUIRE(str6.find('a', ara::fixed_string::npos) == ara::fixed_string::npos);
		REQUIRE(str6.find('a', 1000) == ara::fixed_string::npos);
		REQUIRE(str6.find("") == 0);
		REQUIRE(str6.find("", 2) == 2);
		REQUIRE(str6.find("", 1000) == ara::fixed_string::npos);
		REQUIRE(str6.rfind('a', 0) == ara::fixed_string::npos);
		REQUIRE(str6.rfind(nullptr, 0) == 0);
		REQUIRE(str6.rfind("", 100) == ara::fixed_string::npos);
		REQUIRE(str6.find_first_of("ABC", 100) == ara::fixed_string::npos);
		REQUIRE(str6.find_first_not_of("ABC", 100) == ara::fixed_string::npos);
		REQUIRE(str6.find_last_not_of("ABC", 0) == ara::fixed_string::npos);

		std::string strDummy = str6.str();
		REQUIRE(strDummy.compare(0, strDummy.size(), str6.data(), str6.size()) == 0);
		std::string strDummy2 = str6.as<std::string>();
		REQUIRE(strDummy == strDummy2);

		str2.assign( str6 );
		REQUIRE(str2 == str6);
		std::string strDummy3 = "123456";
		str2.assign(strDummy3);
		REQUIRE(str2 == strDummy3);
		str2 = strDummy2;
		REQUIRE(str2 == strDummy2);

		str2.reset(buf2, buf2 + 2, buf2 + sizeof(buf2));
		REQUIRE(str2.length() == 2);
		REQUIRE(str2.capacity() == 7);
		size_t index = 0;
		for (auto a : str2) {
			if (index == 0)
				REQUIRE(a == 'H');
			else if (index == 1)
				REQUIRE(a == '2');
			++index;
		}
		
		char buf3[] = "abcdefghijk";
		str2.reset(buf3, buf3 + sizeof(buf3) - 1, buf3 + sizeof(buf3));
		REQUIRE(str2 == "abcdefghijk");
		str2.erase(2, 3);
		REQUIRE(str2 == "abfghijk");
		str2.erase(6);
		REQUIRE(str2 == "abfghi");
		str2.erase(8);
		REQUIRE(str2 == "abfghi");

		str2 += std::string("890");
		REQUIRE(str2 == "abfghi890");
		str2.swap(str6);
		REQUIRE(str6 == "abfghi890");
		REQUIRE(str2 == strDummy);
		str2.clear();
		REQUIRE(str2.empty());

		char buf4[16];
		str2.reset(buf4, buf4, buf4 + sizeof(buf4));
		str2 = "1234";
		REQUIRE(str2 == "1234");
		REQUIRE(str2.insert(0, "abc") == "abc1234");
		REQUIRE(str2.insert(4, "abc") == "abc1abc234");
		REQUIRE(str2.insert(ara::fixed_string::npos, "a") == "abc1abc234a");
		REQUIRE(str2.insert(1000, "a") == "abc1abc234aa");
		REQUIRE(str2.insert(1000, "12345") == "abc1abc234aa1234");
		REQUIRE(str2.insert(2, "ABCD") == "abABCDc1abc234aa");
		str2.reserve(10);
		str2.resize(2);
		REQUIRE(str2 == "ab");
		str2.resize(4, 'a');
		REQUIRE(str2 == "abaa");
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
