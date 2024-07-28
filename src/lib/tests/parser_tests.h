#pragma once

#include <catch2/catch_test_macros.hpp>

#include "tdg/json/parser.h"
#include "tdg/json/printer.h"


TEST_CASE("Parsing JSON")
{
	using namespace tdg::json;

	std::ostringstream oss;
	printer<std::scientific, 3> json_printer(oss);
	parser json_parser;

	SECTION("Parsing basic values", "[parser tests]")
	{
		std::string s = R"(["abcd",12,-8,1.234e-03,true,false,null])";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_array());

		const auto& arr = result.get<array>();

		REQUIRE(arr.size() == 7u);
		REQUIRE(arr[0].get<std::string>() == "abcd");
		REQUIRE(arr[1].get<uint64_t>() == 12u);
		REQUIRE(arr[2].get<int64_t>() == -8);
		REQUIRE(arr[3].is_double());
		REQUIRE(arr[4].get<bool>() == true);
		REQUIRE(arr[5].get<bool>() == false);
		REQUIRE(arr[6].is_null());

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Empty array", "[parser tests]")
	{
		std::string s = R"([])";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_array());

		const auto& arr = result.get<array>();

		REQUIRE(arr.size() == 0u);

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Nested empty array", "[parser tests]")
	{
		std::string s = R"([[[]]])";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_array());

		const auto& arr = result.get<array>();

		REQUIRE(arr.size() == 1u);
		REQUIRE(arr[0].is_array());
		REQUIRE(arr[0].get<array>().size() == 1u);
		REQUIRE(arr[0][0].is_array());
		REQUIRE(arr[0][0].get<array>().size() == 0);

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Empty object", "[parser tests]")
	{
		std::string s = R"({})";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_object());

		const auto& obj = result.get<object>();

		REQUIRE(obj.size() == 0u);

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Nested empty object", "[parser tests]")
	{
		std::string s = R"({"abc":{}})";

		const auto result = json_parser.parse(s);
		REQUIRE(result.is_object());

		const auto& obj = result.get<object>();

		REQUIRE(obj.size() == 1u);
		REQUIRE(obj.at("abc").is_object());
		REQUIRE(result["abc"].is_object());

		REQUIRE_THROWS(result["xyz"]);

		const auto& x = result["abc"];
		REQUIRE(x.is_object());

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Object with multiple items", "[parser tests]")
	{
		std::string s = R"({"a":2,"b":5})";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_object());

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Parsing string with \\", "[parser tests]")
	{
		std::string s = R"("c:\Program Files\AMD")";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_string());

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}
}

TEST_CASE("Parsing doubles", "[parser tests]")
{
    using namespace tdg::json;

    parser json_parser;

    auto json_string = GENERATE(
        R"(0.00)",                  \
        R"(2.05)",                  \
        R"(2.999e2)",               \
        R"(2.999e+2)",              \
        R"(2.999e-2)",              \
        R"(2.999E2)",               \
        R"(2.999E+2)",              \
        R"(2.999E-2)",              \
        R"(0.999e-2)",              \
        R"(-0.00)",                 \
        R"(-2.05)",                 \
        R"(-2.999e2)",              \
        R"(-2.999e+2)",             \
        R"(-2.999e-2)",             \
        R"(-2.999E2)",              \
        R"(-2.999E+2)",             \
        R"(-2.999E-2)",             \
        R"(-0.999e-2)"              \
    );

    CAPTURE(json_string);

	REQUIRE_NOTHROW(json_parser.parse(json_string));
}
