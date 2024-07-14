#pragma once

#include <stdexcept>
#include <source_location>

#include "tdg/util/basic_macros.h"


namespace tdg::eh
{
    template <typename... Ts>
    inline std::string make_error_msg(std::source_location location, Ts&&... args)
    {
        std::stringstream ss;

        ss << "Exception in "
            << location.file_name() << ':' << location.line()
            << " [" << location.function_name() << ']'
            << " - ";
        (ss << ... << args) << '\n';

        return ss.str();
    }
}

namespace tdg::json
{
    class invalid_json_exception : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class duplicate_key_exception : public invalid_json_exception
    {
    public:
        using invalid_json_exception::invalid_json_exception;
    };

    class incompatible_assignment_exception : public invalid_json_exception
    {
    public:
        using invalid_json_exception::invalid_json_exception;
    };

}

#define MAKE_ERROR_MSG_IMPL(...) tdg::eh::make_error_msg(__VA_ARGS__)
#define MAKE_ERROR_MSG(...) MAKE_ERROR_MSG_IMPL(std::source_location::current(), __VA_ARGS__)

