#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
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
	class value final
	{
		enum class constant : std::size_t
		{
			JSON_FALSE = 0u,
			JSON_TRUE,
			JSON_NULL
		};

		using internal_value_type = std::variant<object, array, std::string, uint64_t, int64_t, double, constant>;

	public:
		value() : m_value(constant::JSON_NULL) { COUT("value default ctor"); }
		value(const value& other) : m_value(other.m_value) { COUT("value copy ctor"); }
		value(value&& other) noexcept : m_value(std::move(other.m_value)) { COUT("value move ctor"); }

		~value() { COUT("value dtor"); }

		value& operator=(const value& v) = default;
		value& operator=(value&& v) = default;

		//template <typename U, typename... Ts, typename X = std::enable_if_t<sizeof...(Ts) != 0> >
		template <typename U, typename... Ts>
		value(U&& first, Ts&&... rest) requires requires(Ts... rest) { requires sizeof...(Ts) > 0; }
		{
			constexpr auto args_count = sizeof...(Ts) + 1;

			// Optimization for (string, value) pair of parameters
			if constexpr (args_count == 2u && std::is_constructible_v<std::string, std::decay_t<U> >)
			{
				auto obj = object();
				obj.emplace(std::string(std::forward<U>(first)), std::forward<Ts>(rest)...);
				m_value = std::move(obj);
				return;
			}
			else
			{
				auto arr = array();
				arr.reserve(args_count);
				expand(arr, std::forward<U>(first), std::forward<Ts>(rest)...);

				if constexpr ((args_count) % 2 == 0)
				{
					auto is_object = true;

					for (auto iter = arr.begin(); iter != arr.end(); iter += 2)
					{
						if (!iter->is_string())
						{
							is_object = false;
							break;
						}
					}

					if (is_object)
					{
						m_value = object();
						auto& obj = std::get<object>(m_value);

						for (auto iter = arr.begin(); iter != arr.end(); iter += 2)
						{
							assert(iter + 1 != arr.end());
							obj.emplace(
								std::move(std::get<std::string>(iter->m_value)),
								std::move(*(iter + 1))
							);
						}
					}
					else
					{
						m_value = std::move(arr);
					}
				}
				else
				{
					m_value = std::move(arr);
				}
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
			if constexpr (std::is_same_v<std::decay<T>, bool>)
			{
				m_value = (arg ? constant::JSON_TRUE : constant::JSON_FALSE);
			}
			else if constexpr (std::is_unsigned_v<std::decay<T> >)
			{
				m_value = static_cast<uint64_t>(arg);
			}
			else
			{
				m_value = arg;
			}
		}

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
			using visit_func = std::function<void(const value_visitor&, const internal_value_type&)>;

			static std::array<visit_func, 7> visitor_wrapper_array{
				[](const value_visitor& visitor, const internal_value_type& v) { visitor.visit(std::get<object>(v)); },
				[](const value_visitor& visitor, const internal_value_type& v) { visitor.visit(std::get<array>(v)); },
				[](const value_visitor& visitor, const internal_value_type& v) { visitor.visit(std::get<std::string>(v)); },
				[](const value_visitor& visitor, const internal_value_type& v) { visitor.visit(std::get<uint64_t>(v)); },
				[](const value_visitor& visitor, const internal_value_type& v) { visitor.visit(std::get<int64_t>(v)); },
				[](const value_visitor& visitor, const internal_value_type& v) { visitor.visit(std::get<double>(v)); },
				[](const value_visitor& visitor, const internal_value_type& v) {
					auto c = std::get<constant>(v);
					c == constant::JSON_NULL
						? visitor.visit(nullptr)
						: visitor.visit(c == constant::JSON_TRUE);
				}
			};

			visitor_wrapper_array[m_value.index()](visitor, m_value);
		}

		template <typename T>
		value& operator[](T&& key)
			requires std::is_same_v<std::decay_t<T>, std::string> ||
					 std::is_constructible_v<std::string, std::decay_t<T>>
		{
			return std::get<object>(m_value)[std::forward<T>(key)];
		}

		value& operator[](std::size_t index)
		{
			return std::get<array>(m_value)[index];
		}

		const value& operator[](std::size_t index) const
		{
			return std::get<array>(m_value)[index];
		}

	private:

		template <typename U, typename... Ts>
		void expand(array& arr, U&& first, Ts&&... rest)
		{
			arr.emplace_back(value(std::forward<U>(first)));
			expand(arr, std::forward<Ts>(rest)...);
		}

		template <typename U>
		void expand(array& arr, U&& last)
		{
			arr.emplace_back(value(std::forward<U>(last)));
		}

	private:

		internal_value_type m_value;
	};
}