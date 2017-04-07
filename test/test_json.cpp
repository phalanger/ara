
#include "3rd/Catch/single_include/catch.hpp"
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
	}

	SECTION("ref") {
		ara::var	v;

		std::string str1 = "100";

		v = ara::json::parse_ref("100");
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
	}
}