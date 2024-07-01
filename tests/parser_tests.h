#pragma once

#include <catch2/catch_test_macros.hpp>
#include "parser.h"
#include "printer.h"


TEST_CASE("Parsing JSON")
{
	using namespace tdg::json;

	std::ostringstream oss;
	printer<std::scientific, 3> json_printer(oss);
	parser json_parser;

	SECTION("Parsing basic values")
	{
		std::string s = R"(["abcd",  12,  -8, 1.234e-03, true, false, null])";

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

		for (auto pos = s.find(' '); pos != std::string::npos; pos = s.find(' ', pos))
		{
			s.replace(pos, 1u, "");
		}

		REQUIRE(s == oss.str());
	}

	SECTION("Empty array")
	{
		std::string s = R"([])";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_array());

		const auto& arr = result.get<array>();

		REQUIRE(arr.size() == 0u);

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Empty object")
	{
		std::string s = R"({})";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_object());

		const auto& arr = result.get<object>();

		REQUIRE(arr.size() == 0u);

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}

	SECTION("Extra comma")
	{
		std::string s = R"([[2], 1, null]])";

		REQUIRE_NOTHROW(json_parser.parse(s));
	}

	SECTION("Path parsing with \\")
	{
		std::string s = R"("c:\Program Files\AMD")";

		auto result = json_parser.parse(s);
		REQUIRE(result.is_string());

		json_printer.print(result);

		REQUIRE(s == oss.str());
	}
}

