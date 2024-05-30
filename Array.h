#pragma once

#include <vector>
#include <stdexcept>

#include "Value.h"

namespace tdg::json
{
	class array
	{
	public:
		using storage_t = std::vector<value>;

		array() = default;
		array(array&&) = default;
		array(const array&) = delete;

		array(storage_t&& data)
			: m_items(std::move(data))
		{
		}

		void push_back(value&& val)
		{
			m_items.emplace_back(std::move(val));
		}

		const value& operator[](std::size_t i)
		{
			return m_items.at(i);
		}

		size_t size() const
		{
			return m_items.size();
		}

	private:
		storage_t m_items;
	};
}
