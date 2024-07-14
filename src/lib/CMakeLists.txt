cmake_minimum_required (VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("libtdgjson")

add_library(libtdgjson INTERFACE)
target_include_directories(libtdgjson INTERFACE include/)
target_compile_features(libtdgjson INTERFACE cxx_std_20)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_Declare(
  cmake-modules
  GIT_REPOSITORY https://github.com/bilke/cmake-modules.git
  GIT_TAG        master
)

FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(cmake-modules)

add_executable(tests)
target_sources(tests PRIVATE
    tests/test.cpp
    tests/parser_tests.h
    tests/value_tests.h
    tests/negative_parser_tests.h)
target_include_directories(tests PRIVATE ./)
target_link_libraries(tests PUBLIC libtdgjson)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_compile_features(tests PRIVATE cxx_std_20)
target_compile_options(tests PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:CLANG>>: -Wall -Wextra -Werror -Wpedantic -Wconversion -Wsign-conversion> $<$<CXX_COMPILER_ID:MSVC>: /WX /W4>)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(CTest)
include(Catch)
catch_discover_tests(tests)

if (CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_MODULE_PATH ${cmake-modules_SOURCE_DIR})

    include(CodeCoverage)
    append_coverage_compiler_flags()

    # we need to turn off optimization for non-skewed coverage reports
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")

    setup_target_for_coverage_gcovr_xml(
            NAME tests_coverage_xml
            EXECUTABLE tests
            DEPENDENCIES tests
            EXCLUDE "out/*" "tests/*"
    )
    # Works
    setup_target_for_coverage_gcovr_html(
            NAME tests_coverage_html
            EXECUTABLE tests
            DEPENDENCIES tests
            EXCLUDE "out/*" "tests/*"
    )
endif()

