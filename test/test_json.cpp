
#include "3rd/Catch/single_include/catch.hpp"
#include "ara/json.h"

TEST_CASE("json", "[base]") {

	SECTION("base") {

		auto doc = ara::jsvar::init();
		ara::jsvar		var(doc);
		REQUIRE(var.is_null());
	}

	SECTION("parse1") {
		auto doc = ara::jsvar::parse("{\"a\": 100}");
		ara::jsvar	var(doc);
		REQUIRE(var.is_object());
		REQUIRE(var.member_count() == 1);
		REQUIRE(var.has_member("a"));
	}

	SECTION("parse2") {
		std::string strContent = "{\"a\": 100}";
		auto doc = ara::jsvar::parse(strContent);

		ara::jsvar	var(doc);
		REQUIRE(var.is_object());
		REQUIRE(var.member_count() == 1);
	}

	SECTION("parse insitu") {
		std::string strContent = "{\"a\": 100}";
		auto doc = ara::jsvar::parse(std::move(strContent));

		ara::jsvar	var(doc);
		REQUIRE(var.is_object());
		REQUIRE(var.member_count() == 1);
	}
}