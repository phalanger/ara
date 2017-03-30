
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/variant.h"
#include "ara/utils.h"

TEST_CASE("test variant", "[base]") {

	ara::var a;

	REQUIRE( a.is_null() );
	REQUIRE( ara::static_empty<ara::var>::val.is_null() );

	size_t specsize = sizeof(void *) * 2;
	size_t varsize = sizeof(ara::var);

	REQUIRE(varsize == specsize);

	{
		ara::var a = true;
		REQUIRE(a.is_bool());
		REQUIRE(a.get_bool());
		a.get_bool_modify() = false;
		REQUIRE_FALSE(a.get_bool());
	}

	{
		ara::var a = 123;
		REQUIRE(a.is_int());
		REQUIRE(a.get_int() == 123);
		REQUIRE_THROWS_WITH(a.get_int64(), "type is INT not INT64");
		a.get_int_modify() = -456;
		REQUIRE(a.get_int() == -456);
	}

	{
		ara::var a = -345;
		REQUIRE(a.is_int());
		REQUIRE(a.get_int() == -345);
	}

	{
		ara::var a = int64_t(100000);
		REQUIRE(a.is_int64());
		REQUIRE_THROWS_WITH(a.get_int(), "type is INT64 not INT");
		a.get_int64_modify() = -4560000;
		REQUIRE(a.get_int64() == -4560000);
	}

	{
		ara::var a = 0.7;
		REQUIRE(a.is_double());
		REQUIRE(a.get_double() == Approx(0.7));
		a.get_double_modify() = 0.8;
		REQUIRE(a.get_double() == Approx(0.8));
	}

	{
		ara::var a = "hello world";
		REQUIRE(a.is_string());
		REQUIRE(a.is_ref_string());
		REQUIRE_FALSE(a.is_std_string());
		REQUIRE(a.get_string() == "hello world");
		a.get_string_modify() = "hi";
		REQUIRE(a.is_std_string());
		REQUIRE_FALSE(a.is_ref_string());
		REQUIRE(a.get_string() == "hi");
	}

	{
		ara::var a = std::string("hello world");
		REQUIRE(a.is_string());
		REQUIRE(a.is_std_string());
		REQUIRE_FALSE(a.is_ref_string());
		a.get_string_modify() = "hi";
		REQUIRE(a.is_std_string());
		REQUIRE_FALSE(a.is_ref_string());
		REQUIRE(a.get_string() == "hi");
	}

	{
		ara::var a = { 1 , 2, 3, 0.7, "Hello", { 9, 8, 0.5, ara::var("key1", "strval")("key2", 100) } };
		REQUIRE(a.is_array());
		REQUIRE(a.get_array().size() == 6);
		REQUIRE(a.array_size() == 6);
		const ara::var & b = a;
		REQUIRE(b[0].is_int());
		REQUIRE(b[3].is_double());
		REQUIRE(b[4].is_string());
		REQUIRE(b[5].is_array());
		REQUIRE(b[5][1].is_int());
		REQUIRE(b[5][3].is_dict());
	}

	{
		ara::var	a;
		a["key1"] = 1;
		REQUIRE(a.is_dict());
		REQUIRE(a.dict_size() == 1);
		REQUIRE(a["key1"].is_int());
		REQUIRE(a["key2"].is_null());
	}

	{
		ara::var a = ara::var("key1", 1)("key2", 2);
		REQUIRE(a.is_dict());
		REQUIRE(a.dict_size() == 2);
	}
}