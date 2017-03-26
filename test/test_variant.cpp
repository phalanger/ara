
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

}