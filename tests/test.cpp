#pragma once 

#include <catch2/catch_test_macros.hpp>
#include "value.h"


TEST_CASE( "Null by default", "[value]" )
{
	tdg::json::value v;
	REQUIRE(v.is_null());
}

TEST_CASE( "Simple object", "[value]" )
{
	tdg::json::value v{ "abc", 2 };
	REQUIRE(v.is_object());
}

TEST_CASE( "Simple array", "[value]" )
{
	tdg::json::value v{1ul};
	REQUIRE(v.is_unsigned_integer());
}