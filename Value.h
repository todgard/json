#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <cassert>

namespace tdg::json
{
	class value;

	using object = std::unordered_map<std::string, value>;
	using array = std::vector<value>;

	class value
	{
		enum class constant : std::size_t
		{
			JSON_FALSE = 0u,
			JSON_TRUE,
			JSON_NULL
		};

	public:
		using value_list = std::initializer_list<value>;

		value() : m_value(object()) {}

		value(value_list values)
		{
			assert(values.size() > 0);
			// check for potential JSON object member
			if (values.size() == 2u && values.begin()->is_string())
			{
				m_value = object{ {std::get<std::string>(values.begin()->m_value), *(values.begin() + 1)} };
				return;
			}

			const auto all_pairs = std::all_of(
				values.begin(),
				values.end(),
				[](const value& v)
				{
					return v.is_object();
				});

			if (all_pairs)
			{
				object final_object;

				for (auto i = values.begin(); i != values.end(); i++)
				{
					auto& temp_obj = std::get<object>(i->m_value);
					final_object.emplace(temp_obj.begin()->first, temp_obj.begin()->second);
				}
				m_value = std::move(final_object);
			}
			else
			{
				m_value = array(std::move(values));
			}
		}

		value(object&& obj) : m_value(std::move(obj)) {}
		value(array&& arr) : m_value(std::move(arr)) {}
		value(std::string&& s) : m_value(std::move(s)) {}
		value(const char* s) : m_value(std::string(s)) {}
		template <std::size_t N>
		value(const char(&p)[N]) : m_value(std::string(p, N)) {}

		value(std::nullptr_t) : m_value(constant::JSON_NULL) {}

		template <typename T>
		value(T arg) requires std::is_integral_v<T> || std::is_floating_point_v<T>
		{
			if constexpr (std::is_same_v<T, bool>)
			{
				m_value = (arg ? constant::JSON_TRUE : constant::JSON_FALSE);
			}
			else
			{
				m_value = arg;
			}
		}
		//value(int64_t i) : m_value(i) {}
		//value(double d) : m_value(d) {}
		//value(bool b) : m_value(b ? constant::JSON_TRUE : constant::JSON_FALSE) {}

		value(constant c) : m_value(c) {}

		constexpr bool is_object() const
		{
			return m_value.index() == 0;
		}

		constexpr bool is_array() const
		{
			return m_value.index() == 1;
		}

		constexpr bool is_string() const
		{
			return m_value.index() == 2;
		}

		constexpr bool is_unsigned_integer() const
		{
			return m_value.index() == 3;
		}

		constexpr bool is_signed_integer() const
		{
			return m_value.index() == 4;
		}

		constexpr bool is_double() const
		{
			return m_value.index() == 5;
		}

		constexpr bool is_null() const
		{
			return m_value.index() == 6 && std::get<constant>(m_value) == constant::JSON_NULL;
		}

		constexpr bool is_boolean() const
		{
			return m_value.index() == 6 && std::get<constant>(m_value) != constant::JSON_NULL;
		}

	private:
		std::variant<object, array, std::string, uint64_t, int64_t, double, constant> m_value;
	};
}