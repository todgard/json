#pragma once

#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_test_macros.hpp>

#include "parser.h"
#include "printer.h"


TEST_CASE("Comma errors", "[negative parser tests]")
{
    using namespace tdg::json;

    parser json_parser;

    auto json_string = GENERATE(
        R"(,)",                     \
        R"("abc",)",                \
        R"("abc", 2, true,)",       \
        R"({,})",                   \
        R"({},)",                   \
        R"({"abc",})",              \
        R"({"abc", null})",         \
        R"({"abc" : ,})",           \
        R"({"abc" : 2,})",          \
        R"({"abc" : 2},)",          \
        R"([,])",                   \
        R"([],)",                   \
        R"([-3,])",                 \
        R"([-3, -2,])",             \
        R"([-3, , -2])",            \
        R"([-3, -2],)"              \
    );

    REQUIRE_THROWS_AS(json_parser.parse(json_string), tdg::json::invalid_json_exception);
}

TEST_CASE("Key-value separator errors", "[negative parser tests]")
{
    using namespace tdg::json;

    parser json_parser;

    auto json_string = GENERATE(
        R"(:)",                     \
        R"(: "abc")",               \
        R"("abc" :)",               \
        R"("abc" : 2)",             \
        R"({:})",                   \
        R"({}:)",                   \
        R"({: "abc"})",             \
        R"({"abc" : null :})",      \
        R"({"abc" : null, :})",     \
        R"({"abc" : 2}:)",          \
        R"({"abc" : : 2})",         \
        R"({"abc" : : 2}:)",        \
        R"([:])",                   \
        R"([]:)",                   \
        R"([-3:])",                 \
        R"([-3: -2])",              \
        R"([-3, -2 :])",            \
        R"([-3, -2, :])",           \
        R"([-3: : -2])",            \
        R"([-3, -2]:)"              \
    );

    REQUIRE_THROWS_AS(json_parser.parse(json_string), tdg::json::invalid_json_exception);
}

TEST_CASE("Array errors", "[negative parser tests]")
{
    using namespace tdg::json;

    parser json_parser;

    auto json_string = GENERATE(
        R"([)",                     \
        R"(])",                     \
        R"([ [])",                  \
        R"([] ])",                  \
        R"([[ [] ])",               \
        R"([ [] ]])",               \
        R"([[], [], [])",           \
        R"(["abc", 2)",             \
        R"(["abc", 2, null] ])",    \
        R"([ ["abc", 2, null])",    \
        R"([{"abc" : [2, null}]])", \
        R"([{"abc" : 2]})",         \
        R"([{"abc" : 2})",          \
        R"([{"abc"], 2]})"          \
        R"({"x" : [})",             \
        R"({"x" : ]})"              \
    );

    CAPTURE(json_string);

    REQUIRE_THROWS_AS(json_parser.parse(json_string), tdg::json::invalid_json_exception);
}

TEST_CASE("Object errors", "[negative parser tests]")
{
    using namespace tdg::json;

    parser json_parser;

    auto json_string = GENERATE(
        R"({)",                     \
        R"(})",                     \
        R"([}])",                   \
        R"([{])",                   \
        R"({"x" : {})",             \
        R"({"x" : }})",             \
        R"({"x" , {}})",            \
        R"({"x" : , "y" : 2})",     \
        R"({"x" : {}} })",          \
        R"({} })",                  \
        R"([{} }])",                \
        R"([{}, {])",               \
        R"([{}, }])",               \
        R"({"x" : [], null : 5})",  \
        R"(["abc", 2)",             \
        R"(["abc", 2, null] ])",    \
        R"([ ["abc", 2, null])",    \
        R"([{"abc" : [2, null}]])", \
        R"([{"abc" : 2]})",         \
        R"([{"abc" : 2])",          \
        R"([{"abc" : 2], 2])"       \
    );

    CAPTURE(json_string);

    REQUIRE_THROWS_AS(json_parser.parse(json_string), tdg::json::invalid_json_exception);
}

