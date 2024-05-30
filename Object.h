#pragma once

#include <unordered_map>
#include <stdexcept>

#include "Value.h"

namespace tdg::json
{
	class member_already_exists : public std::logic_error
	{
	public:
		member_already_exists(const std::string& message)
			: logic_error(message)
		{
		}
	};

	class object
	{
	public:
		using storage_t = std::unordered_map<std::string, value>;

		object() = default;
		object(object&&) = default;
		object(const object&) = delete;

		object(storage_t&& data)
			: m_members(std::move(data))
		{
		}

		bool try_add(std::string&& key, value&& val) noexcept
		{
			return m_members.emplace(std::move(key), std::move(val)).second;
		}

		void add(std::string&& key, value&& val)
		{
			if (!try_add(std::move(key), std::move(val)))
			{
				throw member_already_exists("Member already exists");
			}
		}

		value& operator[](std::string&& key)
		{
			return m_members.emplace(std::move(key), value()).first->second;
		}

		const value& operator[](const std::string& key)
		{
			return m_members[key];
		}

	private:
		storage_t m_members;
	};
}
