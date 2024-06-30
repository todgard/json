#pragma once

#include <catch2/catch_test_macros.hpp>
#include "value.h"


using namespace tdg::json;

TEST_CASE( "Null by default", "[value]" )
{
	value v;
	REQUIRE(v.is_null());
}

TEST_CASE( "Object vs array deduction", "[value]" )
{
	value v{ "abc", 2, "xyz"};
	REQUIRE(v.is_array());
	REQUIRE(v.get<array>().size() == 3u);
	REQUIRE(v[0].get<std::string>() == "abc");
	REQUIRE(v[1].get<int64_t>() == 2);
	REQUIRE(v[2].get<std::string>() == "xyz");

	value v2{ "abc", 2, "xyz", true};
	REQUIRE(v2.is_object());
	REQUIRE(v2.get<object>().size() == 2u);
	REQUIRE(v2["abc"].get<int64_t>() == 2);

	value v3{ "abc", "xyz", "uvw", nullptr};
	REQUIRE(v3.is_object());
}

TEST_CASE( "Single element array has to be created explicitly", "[value]" )
{
	value v{ 2.0f };
	REQUIRE(!v.is_array());
	REQUIRE(v.is_double());

	value v2{ value::make_array(nullptr) };
	REQUIRE(v2.is_array());
	REQUIRE(v2.get<array>().size() == 1u);
	REQUIRE(v2.get<array>()[0].is_null());
}

TEST_CASE( "Array of string key, value pair has to be created explicitly", "[value]" )
{
	value v{"abc", 2};
	REQUIRE(!v.is_array());
	REQUIRE(v.is_object());

	value v2{ value::make_array("abc", 2) };
	REQUIRE(v2.is_array());
	REQUIRE(v2.get<array>().size() == 2u);
}

TEST_CASE( "Simple array", "[value]" )
{
	value v{ "abc", 2u, 3.0, -1, false, nullptr };

	SECTION("Array created successfully")
	{
		REQUIRE(v.is_array());
		REQUIRE(v[0].is_string());
		REQUIRE(v[1].is_unsigned_integer());
		REQUIRE(v[2].is_double());
		REQUIRE(v[3].is_signed_integer());
		REQUIRE(v[4].is_boolean());
		REQUIRE(v[5].is_null());
	}

	SECTION("Using wrong type to get value throws an exception")
	{
		REQUIRE_THROWS_AS(v[0].get<bool>(), std::bad_variant_access);
	}

	SECTION("Checking stored value")
	{
		auto b = v[4].get<bool>();
		REQUIRE(b == false);
	}

	SECTION("Using assignment operator allows to use different underlying types")
	{
		value v2{ 2u };
		v[4] = v2;
		REQUIRE(v[4].is_unsigned_integer());


		v[5] = value("abc", 2.0);
		REQUIRE(v[5].is_object());
	}

	SECTION("Underlying compound types can be referenced")
	{
		auto& s = v[0].get<std::string>();
		REQUIRE(s == "abc");
		s = "def";
		REQUIRE(v[0].get<std::string>() == "def");

		auto& arr = v.get<array>();
		REQUIRE(arr.size() == 6u);
		arr.erase(arr.end() - 1, arr.end());
		REQUIRE(arr.size() == 5u);
		arr.emplace_back("uvw", true);
		REQUIRE(arr.size() == 6u);
		REQUIRE(v[5].is_object());

		auto& obj = v[5].get<object>();
		REQUIRE(obj["uvw"].get<bool>());
		REQUIRE(obj.size() == 1u);
		obj["xyz"] = -1;
		REQUIRE(obj.size() == 2u);
	}

	SECTION("Setter function throws if different types")
	{
		REQUIRE_THROWS_AS(v[4].set("xyz"), incompatible_assignment_exception);

		REQUIRE(v[0].get<std::string>() == "abc");
		REQUIRE_NOTHROW(v[0].set("xyz"));
		REQUIRE(v[0].get<std::string>() == "xyz");
	}

	SECTION("Setter function does not throw if value is null")
	{
		REQUIRE_NOTHROW(v[5].is_null());
		REQUIRE_NOTHROW(v[5].set("xyz"));
		REQUIRE(v[5].get<std::string>() == "xyz");
	}
}

TEST_CASE( "Simple object", "[value]" )
{
	value v{ "abc", 2 };

	SECTION("Simple object creation")
	{
		REQUIRE(v.is_object());
		REQUIRE(v.get<object>().size() == 1u);
		REQUIRE(v.get<object>().contains("abc"));
		REQUIRE(v["abc"].is_signed_integer());
		REQUIRE(v["abc"].get<int64_t>() == 2);
	}

	SECTION("Using [] operator with non-existent key on value holding object inserts new element")
	{
		auto& new_elem = v["xyz"];

		REQUIRE(v.get<object>().size() == 2u);
		REQUIRE(new_elem.is_null());
		new_elem = true;
		REQUIRE(new_elem.is_boolean());
	}
}

TEST_CASE( "Creating object with duplicate keys should throw", "[value]" )
{
	REQUIRE_THROWS_AS(value("abc", 2, "abc", 5), duplicate_key_exception);
}
