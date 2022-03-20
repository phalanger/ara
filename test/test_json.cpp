
#include "3rd/Catch2/catch.hpp"
#include "ara/json.h"

TEST_CASE("json", "[base]") {

	SECTION("base") {
		ara::var	v;

		REQUIRE( ara::json::parse(v, "", 0) );

		v = ara::json::parse("100");
		REQUIRE(v.is_int());
		REQUIRE(v.get_int() == 100);

		v = ara::json::parse("{  \
			\"a\" : 100,				\
			\"b\" : \"hello world\"		\
			}");
		REQUIRE(v.is_dict());
		REQUIRE(v.dict_size() == 2);
		REQUIRE(v["b"].is_string());
		REQUIRE(v["b"].is_std_string());
		REQUIRE(v["b"].get_string() == "hello world");

		v = "[{\"abcdefghijk\" : 0.6, \"1234567890asd\" : \"11111111111111\"}, 100]"_json;
		REQUIRE(v.is_array());
		REQUIRE(v[0].is_dict());
		REQUIRE(v[1].is_int());

		std::wstring str1 = L"[100, 0.5, \"Hello\"]";
		v = ara::json::parse(str1);
		REQUIRE(v.is_array());
		REQUIRE(v.array_size() == 3);
		REQUIRE(v[2].is_string());
		REQUIRE(v[2].get_string() == "Hello");
	}

	SECTION("ref") {
		ara::var	v;

		std::string str1 = "100";

		v = ara::json::parse_ref( str1 );
		REQUIRE(v.is_int());
		REQUIRE(v.get_int() == 100);

		std::string str2 = "{  \
			\"a\" : 100,				\
			\"b\" : \"hello world\"		\
			}";
		v = ara::json::parse_ref( std::move(str2) );
		REQUIRE(v.is_dict());
		REQUIRE(v.dict_size() == 2);
		REQUIRE(v["b"].is_string());
		REQUIRE(v["b"].is_ref_string());
		REQUIRE(v["b"].get_string() == "hello world");
		std::string::size_type p = str2.find('o');
		str2[p] = 'O';
		REQUIRE(v["b"].get_string() == "hellO world");

		std::wstring str3 = L"{  \
			\"a\" : 100,				\
			\"b\" : \"hello world\"		\
			}";
		v = ara::json::parse_ref( std::move(str3) );
		REQUIRE(v.is_dict());
		REQUIRE(v.dict_size() == 2);
		REQUIRE(v["b"].is_string());
		REQUIRE(v["b"].is_std_string());
		REQUIRE(v["b"].get_string() == "hello world");
		std::wstring::size_type p2 = str3.find('o');
		str3[p2] = 'O';
		REQUIRE(v["b"].get_string() == "hello world");	
	}


	SECTION("save") {

		ara::var	v;

		std::string str1 = "100";

		v = ara::json::parse_ref(str1);
		REQUIRE(v.is_int());
		REQUIRE(v.get_int() == 100);

		REQUIRE(ara::json::save<std::string>(v) == "100");

		std::string str2 = "{  \
			\"a\" : 100,				\
			\"b\" : \"hello world\"		\
			}";
		v = ara::json::parse_ref(std::move(str2));
		std::string s2 = ara::json::save<std::string>(v);
		REQUIRE(s2 == "{\"a\":100,\"b\":\"hello world\"}");
		REQUIRE(ara::json::save_to<std::string>(v, s2));
		REQUIRE(s2 == "{\"a\":100,\"b\":\"hello world\"}{\"a\":100,\"b\":\"hello world\"}");

		std::wstring str3 = L"{  \
			\"a\" : 100,				\
			\"b\" : \"hello world\"		\
			}";
		v = ara::json::parse_ref(std::move(str3));
		std::wstring s3 = ara::json::pretty_save<std::wstring>(v);
		std::wstring s3_pretty = L"{\n    \"a\": 100,\n    \"b\": \"hello world\"\n}";
		REQUIRE(s3 == s3_pretty);
	}

	SECTION("overflow") {
		{
			std::string str1 = "2147483648";
			ara::var v = ara::json::parse_ref(std::move(str1));

			int64_t n = v.get_int64();
			if (n < 0)
				n += 0x100000000;
			REQUIRE(n == 0x80000000);
		}
		{
			std::string str1 = "2147483649";
			ara::var v = ara::json::parse_ref(std::move(str1));

			int64_t n = v.get_int64();
			if (n < 0)
				n += 0x100000000;
			REQUIRE(n == 0x80000001);
		}
	}
}