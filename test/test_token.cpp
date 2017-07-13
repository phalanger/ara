#include "3rd/Catch/single_include/catch.hpp"

#include "ara/token.h"

TEST_CASE("token", "[base]") {

	SECTION("base") {

		const char * spec[] = {
			"Hello",
			"world",
			"hi",
			"wel",
			nullptr
		};

		{
			std::string				src("Hello,world;hi,;wel;");
			ara::token_string		token(src, ";,");
			ara::token_string::result_string	res;
			size_t i = 0;
			while (token.next(res)) {
				REQUIRE(res == spec[i++]);
			}
			REQUIRE(spec[i] == nullptr);
		}

		{
			ara::token_string		token("Hello,world;hi,;wel;", ";,");
			ara::token_string::result_string	res;
			size_t i = 0;
			while (token.next(res)) {
				REQUIRE(res == spec[i++]);
			}
			REQUIRE(spec[i] == nullptr);
		}

	}

	SECTION("simple") {

		const char * spec[] = {
			"Hello",
			"world",
			"hi",
			"wel",
			nullptr
		};

		{
			std::string				src("Hello,world,hi,,wel,,");
			ara::token_base<std::string,char>		token(src, ',');
			ara::token_base<std::string, char>::result_string	res;
			size_t i = 0;
			while (token.next(res)) {
				REQUIRE(res == spec[i++]);
			}
			
		}

	}
}