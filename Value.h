#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "jsonfwd.h"
#include "value_visitor.h"


#define COUT(x) (std::cout << x << '\n')

namespace tdg::json
{
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

		value() : m_value(object()) { COUT("value default ctor"); }
		value(const value& other) : m_value(other.m_value) { COUT("value copy ctor"); }
		value(value&& other) : m_value(std::move(other.m_value)) { COUT("value move ctor"); }
		~value() { COUT("value dtor"); }

		value(value_list values)
		{
			COUT("value initializer_list ctor");
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

		value(object&& obj) : m_value(std::move(obj)) {COUT("value object&& ctor");}
		value(array&& arr) : m_value(std::move(arr)) {COUT("value array&& move ctor");}
		value(std::string&& s) : m_value(std::move(s)) {COUT("value string&& move ctor");}
		value(const char* s) : m_value(std::string(s)) {COUT("value const char* ctor");}
		template <std::size_t N>
		value(const char(&p)[N]) : m_value(std::string(p, N)) {COUT("value const char&[] ctor");}

		value(std::nullptr_t) : m_value(constant::JSON_NULL) {COUT("value nullptr_t ctor");}

		template <typename T>
		value(T arg) requires std::is_integral_v<T> || std::is_floating_point_v<T>
		{
			COUT("value int/bool/double ctor");
			if constexpr (std::is_same_v<T, bool>)
			{
				m_value = (arg ? constant::JSON_TRUE : constant::JSON_FALSE);
			}
			else
			{
				m_value = arg;
			}
		}

		value(constant c) : m_value(c) {COUT("value constant ctor");}

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

		void accept(const value_visitor& visitor) const
		{
			switch (m_value.index())
			{
			case 0:
				visitor.visit(std::get<object>(m_value));
				break;
			case 1:
				visitor.visit(std::get<array>(m_value));
				break;
			case 2:
				visitor.visit(std::get<std::string>(m_value));
				break;
			case 3:
				visitor.visit(std::get<uint64_t>(m_value));
				break;
			case 4:
				visitor.visit(std::get<int64_t>(m_value));
				break;
			case 5:
				visitor.visit(std::get<double>(m_value));
				break;
			case 6:
				if (auto c = std::get<constant>(m_value); c == constant::JSON_NULL)
				{
					visitor.visit(nullptr);
				}
				else
				{
					visitor.visit(c == constant::JSON_TRUE);
				}
				break;
			default:
				break;
			}
		}

	private:

		std::variant<object, array, std::string, uint64_t, int64_t, double, constant> m_value;
	};
}