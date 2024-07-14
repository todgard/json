#pragma once

#include <map>
#include <vector>

namespace tdg::json
{
	class value;
	using array = std::vector<value>;
	using object = std::map<std::string, value>;
}
