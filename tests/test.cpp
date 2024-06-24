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
	REQUIRE(v.get<tdg::json::object>().size() == 1u);
	REQUIRE(v.get<tdg::json::object>().contains("abc"));
	REQUIRE(v["abc"].is_signed_integer());
	REQUIRE(v["abc"].get<int64_t>() == 2);
}

TEST_CASE( "Object vs array deduction", "[value]" )
{
	tdg::json::value v{ "abc", 2, "xyz"};
	REQUIRE(v.is_array());

	tdg::json::value v2{ "abc", 2, "xyz", true};
	REQUIRE(v2.is_object());

	tdg::json::value v3{ "abc", "xyz", "uvw", nullptr};
	REQUIRE(v3.is_object());
}

TEST_CASE( "Single element array has to be forced", "[value]" )
{
	tdg::json::value v{ tdg::json::value::make_array(nullptr) };
	REQUIRE(v.is_array());
	REQUIRE(v.get<tdg::json::array>().size() == 1u);
	REQUIRE(v.get<tdg::json::array>()[0].is_null());
}

TEST_CASE( "Array of string key, value pair has to be forced", "[value]" )
{
	tdg::json::value v{ tdg::json::value::make_array("abc", 2) };
	REQUIRE(v.is_array());
	REQUIRE(v.get<tdg::json::array>().size() == 2u);
}

TEST_CASE( "Simple array", "[value]" )
{
	tdg::json::value v{ "abc", 2u, 3.0, -1, false, nullptr };
	REQUIRE(v.is_array());
	REQUIRE(v[0].is_string());
	REQUIRE(v[1].is_unsigned_integer());
	REQUIRE(v[2].is_double());
	REQUIRE(v[3].is_signed_integer());
	REQUIRE(v[4].is_boolean());
	REQUIRE(v[5].is_null());
	//TODO: fix for bool, nullptr not returned by reference
	//auto b = v[4].get<bool>();
}