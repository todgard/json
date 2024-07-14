cmake_minimum_required (VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("json")

add_subdirectory(src/lib)

add_executable (json json.cpp json.h)
target_compile_features(tests PRIVATE cxx_std_20)
target_compile_options(json PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:CLANG>>: -Wall -Wextra -Werror> $<$<CXX_COMPILER_ID:MSVC>: /WX /W4>)
target_link_libraries(json PUBLIC libtdgjson)

