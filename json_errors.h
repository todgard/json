#pragma once

#include <stdexcept>


namespace tdg::eh
{
	template <typename... Ts>
	inline std::string make_error_msg(Ts&&... args)
	{
		std::stringstream ss;

		(ss << ... << args) << '\n';

		return ss.str();
	}
}

namespace tdg::json
{
	class duplicate_key_exception : public std::runtime_error
	{
	public:
		explicit duplicate_key_exception(const std::string& msg) : std::runtime_error(msg) {}
	};

	class incompatible_assignment_exception : public std::runtime_error
	{
	public:
		explicit incompatible_assignment_exception(const std::string& msg) : std::runtime_error(msg) {}
	};

	// Parsing exceptions
	class invalid_json_exception : public std::runtime_error
	{
	public:
		explicit invalid_json_exception(const std::string& msg) : std::runtime_error(msg) {}
	};
}

