#include <catch2/catch_test_macros.hpp>
#include "value.h"


using namespace tdg::json;

TEST_CASE( "Null by default", "[value]" )
{
	value v;
	REQUIRE(v.is_null());
}

TEST_CASE( "Simple object", "[value]" )
{
	value v{ "abc", 2 };
	REQUIRE(v.is_object());
	REQUIRE(v.get<object>().size() == 1u);
	REQUIRE(v.get<object>().contains("abc"));
	REQUIRE(v["abc"].is_signed_integer());
	REQUIRE(v["abc"].get<int64_t>() == 2);
}

TEST_CASE( "Object vs array deduction", "[value]" )
{
	value v{ "abc", 2, "xyz"};
	REQUIRE(v.is_array());

	value v2{ "abc", 2, "xyz", true};
	REQUIRE(v2.is_object());

	value v3{ "abc", "xyz", "uvw", nullptr};
	REQUIRE(v3.is_object());
}

TEST_CASE( "Single element array has to be forced", "[value]" )
{
	value v{ value::make_array(nullptr) };
	REQUIRE(v.is_array());
	REQUIRE(v.get<array>().size() == 1u);
	REQUIRE(v.get<array>()[0].is_null());
}

TEST_CASE( "Array of string key, value pair has to be forced", "[value]" )
{
	value v{ value::make_array("abc", 2) };
	REQUIRE(v.is_array());
	REQUIRE(v.get<array>().size() == 2u);
}

TEST_CASE( "Simple array", "[value]" )
{
	value v{ "abc", 2u, 3.0, -1, false, nullptr };
	REQUIRE(v.is_array());
	REQUIRE(v[0].is_string());
	REQUIRE(v[1].is_unsigned_integer());
	REQUIRE(v[2].is_double());
	REQUIRE(v[3].is_signed_integer());
	REQUIRE(v[4].is_boolean());
	REQUIRE(v[5].is_null());

	REQUIRE_THROWS_AS(v[0].get<bool>(), std::bad_variant_access);

	auto b = v[4].get<bool>();
	REQUIRE(b == false);

	value v2{2u};
	v[4] = v2;
	REQUIRE(v[4].is_unsigned_integer());

	auto& s = v[0].get<std::string>();
	s = "def";
	REQUIRE(v[0].get<std::string>() == "def");

	REQUIRE_THROWS_AS(v[5].set("xyz"), incompatible_assignment_exception);

	v[5] = value("abc", 2.0);
	REQUIRE(v[5].is_object());
}