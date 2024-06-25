#pragma once

#include <stdexcept>

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
}

