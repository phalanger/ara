
#include "3rd/Catch2/catch.hpp"

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
		REQUIRE(str2.length() == 9);
		REQUIRE(str2 == "aaa123456");
		REQUIRE(str2.ref_count() == 2);

		str2 = std::string("Hello");
		REQUIRE(str2.size() == 5);
		REQUIRE(str2 == std::string("Hello"));
		str2.clear();
		REQUIRE(str2.size() == 0);
		REQUIRE(str2.empty());
		str2.swap(str1);
		REQUIRE(str2.size() == 9);
		REQUIRE(str2.data_type() == ara::key_string::TYPE_STORE);
		REQUIRE(str2 == "aaa123456");

		ara::key_string str3;
		REQUIRE(str3.empty());
		REQUIRE(str3.data_type() == ara::key_string::TYPE_CONST);

		std::string tmp("123456789");
		ara::key_string str4(tmp);
		REQUIRE(str4.data_type() == ara::key_string::TYPE_STORE);
		ara::key_string str5(ara::key_string::ref(tmp));
		REQUIRE(str5.data_type() == ara::key_string::TYPE_CONST);
		REQUIRE(str4 == str5);
		ara::key_string str5_2(ara::key_string::copy(tmp));
		REQUIRE(str5_2.data_type() == ara::key_string::TYPE_STORE);
		REQUIRE(str4 == str5_2);

		const char * tmp2 = "H22323";
		ara::key_string		str6 = ara::key_string::copy(tmp2);
		REQUIRE(str6 == tmp2);
		REQUIRE(str6.data_type() == ((sizeof(void *) >= 6 ? ara::key_string::TYPE_BUF : ara::key_string::TYPE_STORE)));
		ara::key_string		str6_2 = ara::key_string::ref(tmp2);
		REQUIRE(str6_2.data_type() == ((sizeof(void *) >= 6 ? ara::key_string::TYPE_BUF : ara::key_string::TYPE_CONST)));

		REQUIRE(str6_2 == tmp2);
		const char * tmp3 = "H2232378787123123";
		ara::key_string		str6_3 = ara::key_string::copy(tmp3);
		REQUIRE(str6_3.data_type() == ara::key_string::TYPE_STORE);
		ara::key_string		str6_4 = ara::key_string::ref(tmp3);
		REQUIRE(str6_4.data_type() == ara::key_string::TYPE_CONST);

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
		REQUIRE(str6.find("A", 3) == ara::key_string::npos);
		REQUIRE(str6.find(std::string("23"), 2) == 2);
		REQUIRE(str6.rfind('2') == 4);
		REQUIRE(str6.rfind('3', 4) == 3);
		REQUIRE(str6.rfind("H2") == 0);
		REQUIRE(str6.rfind("23") == 4);
		REQUIRE(str6.rfind("23", 3) == 2);
		REQUIRE(str6.rfind("A", 3) == ara::key_string::npos);
		REQUIRE(str6.rfind(std::string("23"), 3) == 2);

		REQUIRE(str6.find_first_of("BHA") == 0);
		REQUIRE(str6.find_first_of("BHA", 0) == 0);
		REQUIRE(str6.find_first_of("BAC", 0) == ara::key_string::npos);
		REQUIRE(str6.find_first_of("2345", 2) == 2);
		REQUIRE(str6.find_first_of(std::string("2345"), 2) == 2);
		REQUIRE(str6.find_first_not_of("abc", 0) == 0);
		REQUIRE(str6.find_first_not_of("BHA", 0) == 1);
		REQUIRE(str6.find_first_not_of("H23", 0) == ara::key_string::npos);
		REQUIRE(str6.find_first_not_of("H246", 2) == 3);
		REQUIRE(str6.find_first_not_of(std::string("H246"), 2) == 3);

		REQUIRE(str6.find_last_of("BHA") == 0);
		REQUIRE(str6.find_last_of("BHA", 0) == ara::key_string::npos);
		REQUIRE(str6.find_last_of("BAC", 0) == ara::key_string::npos);
		REQUIRE(str6.find_last_of("2345") == 5);
		REQUIRE(str6.find_last_of("2345", 2) == 1);
		REQUIRE(str6.find_last_of(std::string("2345"), 2) == 1);
		REQUIRE(str6.find_last_not_of("abc") == 5);
		REQUIRE(str6.find_last_not_of("BHA", 100) == 5);
		REQUIRE(str6.find_last_not_of("H23") == ara::key_string::npos);
		REQUIRE(str6.find_last_not_of("H246", 4) == 3);
		REQUIRE(str6.find_last_not_of(std::string("H246"), 4) == 3);

		REQUIRE(str6.find('a', ara::key_string::npos) == ara::key_string::npos);
		REQUIRE(str6.find('a', 1000) == ara::key_string::npos);
		REQUIRE(str6.find("") == 0);
		REQUIRE(str6.find("", 2) == 2);
		REQUIRE(str6.find("", 1000) == ara::key_string::npos);
		REQUIRE(str6.rfind('a', 0) == ara::key_string::npos);
		REQUIRE(str6.rfind(nullptr, 0) == 0);
		REQUIRE(str6.rfind("", 100) == ara::key_string::npos);
		REQUIRE(str6.find_first_of("ABC", 100) == ara::key_string::npos);
		REQUIRE(str6.find_first_not_of("ABC", 100) == ara::key_string::npos);
		REQUIRE(str6.find_last_not_of("ABC", 0) == ara::key_string::npos);

		std::string strDummy = str6.str();
		REQUIRE(strDummy.compare(0, strDummy.size(), str6.data(), str6.size()) == 0);
		std::string strDummy2 = str6.as<std::string>();
		REQUIRE(strDummy == strDummy2);
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
		REQUIRE(str1.data_type() ==  (sizeof(wchar_t) * 4 <= sizeof(void *) ? ara::key_wstring::TYPE_BUF : ara::key_wstring::TYPE_STORE));

		str1 = ara::key_wstring::copy(L"aaa123456");
		REQUIRE(str1.size() == 9);
		REQUIRE(str1.data_type() == ara::key_wstring::TYPE_STORE);

		ara::key_wstring str2 = str1;
		REQUIRE(str2.size() == 9);
		REQUIRE(str2 == L"aaa123456");
		REQUIRE(str2.ref_count() == 2);
	}
}
