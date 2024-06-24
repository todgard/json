#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "jsonfwd.h"
#include "json_errors.h"
#include "value_visitor.h"


#define COUT(x) (std::cout << x << '\n')

namespace tdg::json
{
	namespace detail
	{
		template <std::size_t S, typename K, typename... Ts >
		struct is_object_constructible : public std::integral_constant<bool, (std::is_constructible_v<std::string, K> || S % 2u == 1u) && is_object_constructible<S + 1, Ts...>::value >
		{
		};

		template <std::size_t S, typename V>
		struct is_object_constructible<S, V> : public std::integral_constant<bool, (S > 0) && (S % 2u == 1)>
		{
		};
	}

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
		value() { COUT("value default ctor"); }
		value(const value& other) : m_value(other.m_value) { COUT("value copy ctor"); }
		value(value&& other) noexcept : m_value(std::move(other.m_value)) { COUT("value move ctor"); }

		~value() { COUT("value dtor"); }

		value& operator=(const value& v) = default;
		value& operator=(value&& v) = default;

		//template <typename U, typename... Ts, typename X = std::enable_if_t<sizeof...(Ts) != 0> >
		template <typename U, typename... Ts>
		value(U&& first, Ts&&... rest) requires requires(Ts...) { requires sizeof...(Ts) > 0; }
		{
			constexpr auto args_count = sizeof...(Ts) + 1;

			if constexpr (args_count % 2u == 0 && detail::is_object_constructible<0u, U, Ts...>::value)
			{
				m_value = make_object(std::forward<U>(first), std::forward<Ts>(rest)...);
			}
			else
			{
				m_value = make_array(std::forward<U>(first), std::forward<Ts>(rest)...);
			}

			/*
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
				auto arr = make_array(std::forward<U>(first), std::forward<Ts>(rest)...);

				if constexpr (args_count % 2 == 0)
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
							//TODO: add checks/parameterization for duplicated keys
							obj.try_emplace(
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
			*/
		}

		explicit(false) value(object&& obj) : m_value(std::move(obj)) {COUT("value object&& ctor");}
		explicit(false) value(array&& arr) : m_value(std::move(arr)) {COUT("value array&& move ctor");}
		explicit(false) value(std::string&& s) : m_value(std::move(s)) {COUT("value string&& move ctor");}
		explicit(false) value(const char* s) : m_value(std::string(s)) {COUT("value const char* ctor");}
		template <std::size_t N>
		explicit(false) value(const char(&p)[N]) : m_value(std::string(p, N)) {COUT("value const char&[] ctor");}
		explicit(false) value(std::nullptr_t) {COUT("value nullptr_t ctor");}

		template <typename T>
		explicit(false) value(T arg) requires std::is_integral_v<T> || std::is_floating_point_v<T>
		{
			COUT("value int/bool/double ctor");
			if constexpr (std::is_same_v<std::decay_t<T>, bool>)
			{
				m_value = (arg ? constant::JSON_TRUE : constant::JSON_FALSE);
			}
			else if constexpr (std::is_unsigned_v<std::decay_t<T> >)
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
				[](const value_visitor& vtor, const internal_value_type& val) { vtor.visit(std::get<object>(val)); },
				[](const value_visitor& vtor, const internal_value_type& val) { vtor.visit(std::get<array>(val)); },
				[](const value_visitor& vtor, const internal_value_type& val) { vtor.visit(std::get<std::string>(val)); },
				[](const value_visitor& vtor, const internal_value_type& val) { vtor.visit(std::get<uint64_t>(val)); },
				[](const value_visitor& vtor, const internal_value_type& val) { vtor.visit(std::get<int64_t>(val)); },
				[](const value_visitor& vtor, const internal_value_type& val) { vtor.visit(std::get<double>(val)); },
				[](const value_visitor& vtor, const internal_value_type& val) {
					auto c = std::get<constant>(val);
					c == constant::JSON_NULL
						? vtor.visit(nullptr)
						: vtor.visit(c == constant::JSON_TRUE);
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

		template <typename T>
		T& get()
			requires std::is_same_v<object, T> || std::is_same_v<array, T> || std::is_same_v<std::string, T> ||
					 std::is_same_v<uint64_t, T> || std::is_same_v<int64_t, T> || std::is_same_v<double, T> ||
					 std::is_same_v<bool, T> || std::is_same_v<std::nullptr_t, T>
		{
			if constexpr (std::is_same_v<bool, T>)
			{
				return std::get<constant>(m_value) == constant::JSON_TRUE;
			}
			else if constexpr (std::is_same_v<std::nullptr_t, T>)
			{
				return std::get<constant>(m_value) == constant::JSON_NULL ? nullptr : throw std::bad_variant_access();
			}
			else
			{
				return std::get<T>(m_value);
			}
		}

		template <typename T>
		const T& get() const
		{
			return std::get<T>(m_value);
		}

		template <typename T, typename... Ts>
		static constexpr array make_array(T&& first, Ts&&... ts)
		{
			array arr;
			arr.reserve(sizeof...(Ts) + 1);
			arr.emplace_back(std::forward<T>(first));

			(arr.emplace_back(value(std::forward<Ts>(ts))), ...);

			return arr;
		}

	private:

		template <typename K, typename V, typename... Ts>
		static constexpr object make_object(K&& key, V&& val, Ts&&... ts) requires requires(K, Ts...) { requires std::is_constructible_v<std::string, std::decay_t<K> >;  requires sizeof...(Ts) % 2 == 0; }
		{
			object obj;

			make_object(obj, std::forward<K>(key), std::forward<V>(val), std::forward<Ts>(ts)...);

			return obj;
		}

		template <typename K, typename V, typename... Ts>
		static constexpr void make_object(object& obj, K&& key, V&& val, Ts&&... ts)
		{
			if (!obj.try_emplace(std::forward<K>(key), std::forward<V>(val)).second)
			{
				throw duplicate_key_exception("Duplicate key while trying to create json object");
			}

			if constexpr (sizeof...(Ts) > 0)
			{
				make_object(obj, std::forward<Ts>(ts)...);
			}
		}

		internal_value_type m_value = constant::JSON_NULL;
	};
}