#include "3rd/Catch/single_include/catch.hpp"

#include "ara/stringext.h"

TEST_CASE("stringext benchmark", "[.],[benchmark]") {

	const size_t nCount = 1000000;
	char buf[16];

	SECTION("ara::snprintf") {
		for (size_t i = 0; i < nCount; ++i) {
			size_t n = ara::snprintf(buf, 16, "Hello %s %d", "world", 123);
			REQUIRE(n == 15);
		}
	}

	SECTION("snprintf") {
		for (size_t i = 0; i < nCount; ++i) {
			size_t n = ::snprintf(buf, 16, "Hello %s %d", "world", 123);
			REQUIRE(n == 15);
		}
	}

	SECTION("ara::snprintf2") {
		for (size_t i = 0; i < nCount; ++i) {
			size_t n = ara::snprintf(buf, 16, "Hello %s %05d", "world", 123);
			REQUIRE(n == 16);
		}
	}

	SECTION("snprintf2") {
		for (size_t i = 0; i < nCount; ++i) {
			size_t n = ::snprintf(buf, 16, "Hello %s %05d", "world", 123);
			REQUIRE(n == 17);
		}
	}
}
