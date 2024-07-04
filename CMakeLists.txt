# CMakeList.txt : CMake project for json, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("json")

include_directories(./)
# Add source to this project's executable.
add_executable (json "json.cpp" "json.h" "value.h" "printer.h" "value_visitor.h" "pretty_printer.h" "parser.h")
target_compile_options(json PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:CLANG>>: -Wall -Wextra -Werror> $<$<CXX_COMPILER_ID:MSVC>: /WX /W4>)

# TODO: Add tests and install targets if needed.

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests tests/test.cpp "pretty_printer.h" "tests/parser_tests.h" "tests/value_tests.h" "tests/negative_parser_tests.h")
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_compile_options(tests PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:CLANG>>: -Wall -Wextra -Werror> $<$<CXX_COMPILER_ID:MSVC>: /WX /W4>)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

include(CTest)
include(Catch)
catch_discover_tests(tests)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET json tests PROPERTY CXX_STANDARD 20)
endif()

