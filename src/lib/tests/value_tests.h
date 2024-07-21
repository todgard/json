#pragma once

#include <math.h>

#include <catch2/catch_test_macros.hpp>

#include "tdg/json/printer.h"
#include "tdg/json/value.h"


using namespace tdg::json;

TEST_CASE("Null by default", "[value]")
{
    value v;
    REQUIRE(v.is_null());
}

TEST_CASE("Object vs array deduction", "[value]")
{
    value v{ "abc", 2, "xyz"};
    REQUIRE(v.is_array());
    REQUIRE(v.get<array>().size() == 3u);
    REQUIRE(v[0].get<std::string>() == "abc");
    REQUIRE(v[1].get<int64_t>() == 2);
    REQUIRE(v[2].get<std::string>() == "xyz");

    value v1{"abc", 2u};
    REQUIRE(v1.is_object());
    REQUIRE(v1.get<object>().size() == 1u);
    REQUIRE(v1["abc"].is_unsigned_integer());
    REQUIRE(v1["abc"].get<uint64_t>() == 2);

    value v2{ "abc", 2, "xyz", true};
    REQUIRE(v2.is_object());
    REQUIRE(v2.get<object>().size() == 2u);
    REQUIRE(v2["abc"].get<int64_t>() == 2);

    value v3{ "abc", "xyz", "uvw", nullptr};
    REQUIRE(v3.is_object());
}

TEST_CASE("Zero size compound types creation", "[value]")
{
    value v1{array()};
    REQUIRE(v1.is_array());
    REQUIRE(v1.get<array>().size() == 0u);

    value v2{object()};
    REQUIRE(v2.is_object());
    REQUIRE(v2.get<object>().size() == 0u);

    value v3{""};
    REQUIRE(v3.is_string());
    REQUIRE(v3.get<std::string>().empty());
}

TEST_CASE("Single element array has to be created explicitly", "[value]")
{
    value v{ 2.0f };
    REQUIRE(!v.is_array());
    REQUIRE(v.is_double());

    value v2{ value::make_array(nullptr) };
    REQUIRE(v2.is_array());
    REQUIRE(v2.get<array>().size() == 1u);
    REQUIRE(v2.get<array>()[0].is_null());
}

TEST_CASE("Two element array with string as first element has to be created explicitly", "[value]")
{
    value v{"abc", 2};
    REQUIRE(!v.is_array());
    REQUIRE(v.is_object());

    value v2{ value::make_array("abc", 2) };
    REQUIRE(v2.is_array());
    REQUIRE(v2.get<array>().size() == 2u);
}

TEST_CASE("Simple array", "[value]")
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

    SECTION("Using wrong type to get value throws an exception", "[value]")
    {
        REQUIRE_THROWS_AS(v[0].get<bool>(), std::bad_variant_access);
    }

    SECTION("Checking stored value")
    {
        auto b = v[4].get<bool>();
        REQUIRE(b == false);
    }

    SECTION("Using assignment operator allows to use different underlying types", "[value]")
    {
        value v2{ 2u };
        v[4] = v2;
        REQUIRE(v[4].is_unsigned_integer());


        v[5] = value("abc", 2.0);
        REQUIRE(v[5].is_object());
    }

    SECTION("Underlying compound types can be referenced", "[value]")
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
        REQUIRE(obj["xyz"].is_signed_integer());
        REQUIRE(obj["xyz"].get<int64_t>() == -1);

        obj.try_emplace("xyz", 3.0);
        REQUIRE(obj.size() == 2u);
        REQUIRE(obj["xyz"].is_signed_integer());

        obj.insert_or_assign("xyz", 3.0);
        REQUIRE(obj.size() == 2u);
        REQUIRE(obj["xyz"].is_double());
        REQUIRE(std::fabs(obj["xyz"].get<double>() - 3.0) < 0.001);
    }

    SECTION("Setter function throws by default if different types", "[value]")
    {
        REQUIRE_THROWS_AS(v[4].set("xyz"), incompatible_assignment_exception);

        REQUIRE(v[0].get<std::string>() == "abc");
        REQUIRE_NOTHROW(v[0].set("xyz"));
        REQUIRE(v[0].get<std::string>() == "xyz");
    }

    SECTION("Setter function can have same types check disabled", "[value]")
    {
        REQUIRE_NOTHROW(v[5].is_null());
        REQUIRE_THROWS_AS(v[5].set(true), incompatible_assignment_exception);
        REQUIRE_NOTHROW(v[5].set(true, false /* same_type_check */));
        REQUIRE(v[5].get<bool>() == true);

        REQUIRE_THROWS_AS(v[5].set(nullptr), incompatible_assignment_exception);
        REQUIRE_NOTHROW(v[5].set(nullptr, false /* same_type_check */));
        REQUIRE(v[5].is_null());

        REQUIRE_THROWS_AS(v[5].set("xyz"), incompatible_assignment_exception);
        REQUIRE_NOTHROW(v[5].set("xyz", false /* same_type_check */));
        REQUIRE(v[5].get<std::string>() == "xyz");
    }
}

TEST_CASE("Nested empty arrays", "[value]")
{
    value v{
        value::make_array(
            value::make_array(
                value::make_array()))};

    REQUIRE(v.is_array());
    REQUIRE(v.get<array>().size() == 1u);
    REQUIRE(v[0].is_array());
    REQUIRE(v[0].get<array>().size() == 1u);
    REQUIRE(v[0][0].is_array());
    REQUIRE(v[0][0].get<array>().size() == 0u);

    std::stringstream ss;
    printer p(ss);
    p.print(v);

    REQUIRE(ss.str() == R"([[[]]])");
}

TEST_CASE("Collapsing values", "[value]")
{
    value v{
        value(
            value(
                value::make_array()))};

    REQUIRE(v.is_array());
    REQUIRE(v.get<array>().size() == 0u);

    std::stringstream ss;
    printer p(ss);
    p.print(v);

    REQUIRE(ss.str() == R"([])");
}

TEST_CASE("Simple object", "[value]")
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

TEST_CASE("Creating object with duplicate keys should throw", "[value]")
{
    REQUIRE_THROWS_AS(value("abc", 2, "abc", 5), duplicate_key_exception);
}

TEST_CASE("Big example 1", "[value]")
{
    value v{
        value{"abc", 5, "uvw", value(1, 2, 3)},
        value{"dsa", object()},
        value::make_array(true),
        value::make_array("abc", nullptr)
    };

    std::stringstream ss;
    printer p(ss);
    p.print(v);

    REQUIRE(ss.str() == R"([{"abc":5,"uvw":[1,2,3]},{"dsa":{}},[true],["abc",null]])");
}

TEST_CASE("Big example 2", "[value]")
{
    value v{
        value(true, false, true, true),
        1234567890,
        nullptr,
        value("asdf", 5, "adsf", value::make_array(), "ups", value(0.56, 0.3e-10, 0.17e3, -8.99e-2)),
        value(
            value{"abc", 5, "uvw", value(1, 2, 3)},
            value{"dsa", object()},
            value::make_array(true),
            value::make_array("abc", nullptr)
            )
    };

    std::stringstream ss;
    printer<std::scientific, 2u> p(ss);
    p.print(v);

    REQUIRE(ss.str() == R"([[true,false,true,true],1234567890,null,{"adsf":[],"asdf":5,"ups":[5.60e-01,3.00e-11,1.70e+02,-8.99e-02]},[{"abc":5,"uvw":[1,2,3]},{"dsa":{}},[true],["abc",null]]])");
}

TEST_CASE("Value from lvalues", "[value]")
{
    using namespace std::string_literals;

    auto s = "test"s;
    auto i = -1;
    auto u = 2u;
    auto d = 0.5;
    auto b = false;

    value v(s);
    REQUIRE(v.is_string());

    v = i;
    REQUIRE(v.is_signed_integer());

    v = u;
    REQUIRE(v.is_unsigned_integer());

    v = d;
    REQUIRE(v.is_double());

    v = b;
    REQUIRE(v.is_boolean());

    const auto& ri = i;
    v = value(ri);
    REQUIRE(v.is_signed_integer());
}

TEST_CASE("Assigning values creates deep copy", "[value]")
{
    value v{
        value{"abc", 5, "uvw", value(1, 2, 3)},
        value{"dsa", object()},
        value::make_array(true),
        value::make_array("abc", nullptr)
    };

    auto v2 = v;
    v[0] = nullptr;
    REQUIRE(v[0].is_null());
    REQUIRE(v2[0].is_object());

    std::stringstream ss;
    printer p(ss);
    p.print(v);

    REQUIRE(ss.str() == R"([null,{"dsa":{}},[true],["abc",null]])");

    ss.str("");
    p.print(v2);

    REQUIRE(ss.str() == R"([{"abc":5,"uvw":[1,2,3]},{"dsa":{}},[true],["abc",null]])");
}
