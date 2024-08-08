#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "tdg/json/json_errors.h"
#include "tdg/json/jsonfwd.h"
#include "tdg/json/value_visitor.h"


namespace tdg::json
{
    namespace detail
    {
        template <std::size_t S, typename K, typename... Ts >
        struct is_object_constructible : public std::integral_constant<bool, (std::is_constructible_v<std::string, K> || S % 2u == 1u) && is_object_constructible<S + 1, Ts...>::value >
        {
        };

        template <std::size_t S, typename V>
        struct is_object_constructible<S, V> : public std::integral_constant<bool, (S > 0) && (S % 2u == 1u)>
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

        internal_value_type m_value = constant::JSON_NULL;

    public:
        value() = default;
        value(const value& other) = default;
        value(value&& other) noexcept = default;

        ~value() = default;

        value& operator=(const value& v) = default;
        value& operator=(value&& v) = default;

        template <typename U, typename... Ts>
        value(U&& first, Ts&&... rest) requires requires(Ts...) { requires sizeof...(Ts) > 0u; }
        {
            if constexpr (detail::is_object_constructible<0u, U, Ts...>::value)
            {
                m_value = make_object(std::forward<U>(first), std::forward<Ts>(rest)...);
            }
            else
            {
                m_value = make_array(std::forward<U>(first), std::forward<Ts>(rest)...);
            }

        }

        explicit(false) value(object&& obj) : m_value(std::move(obj)) {}
        explicit(false) value(array&& arr) : m_value(std::move(arr)) {}
        explicit(false) value(const std::string& s) : m_value(s) {}
        explicit(false) value(std::string&& s) : m_value(std::move(s)) {}
        explicit(false) value(const char* s) : m_value(std::string(s)) {}
        explicit(false) value(std::nullptr_t) {}

        template <typename T>
        explicit(false) value(T arg) requires std::is_integral_v<T> || std::is_floating_point_v<T>
        {
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

        constexpr bool is_object() const noexcept
        {
            return m_value.index() == 0;
        }

        constexpr bool is_array() const noexcept
        {
            return m_value.index() == 1;
        }

        constexpr bool is_string() const noexcept
        {
            return m_value.index() == 2;
        }

        constexpr bool is_unsigned_integer() const noexcept
        {
            return m_value.index() == 3;
        }

        constexpr bool is_signed_integer() const noexcept
        {
            return m_value.index() == 4;
        }

        constexpr bool is_double() const noexcept
        {
            return m_value.index() == 5;
        }

        constexpr bool is_null() const noexcept
        {
            return m_value.index() == 6 && std::get<constant>(m_value) == constant::JSON_NULL;
        }

        constexpr bool is_boolean() const noexcept
        {
            return m_value.index() == 6 && std::get<constant>(m_value) != constant::JSON_NULL;
        }

        constexpr bool has_same_type(const value& other) const noexcept
        {
            return m_value.index() == other.m_value.index() && is_boolean() == other.is_boolean();
        }

        void accept(value_visitor& visitor) const
        {
            std::visit(
                [&visitor]<typename T>(const T& arg)
                {
                    if constexpr (std::is_same_v<constant, T>)
                    {
                        arg == constant::JSON_NULL
                            ? visitor.visit(nullptr)
                            : visitor.visit(arg == constant::JSON_TRUE);
                    }
                    else
                    {
                        visitor.visit(arg);
                    }
                },
                m_value
            );
        }

        // Object elements insertion/access
        template <typename T>
        value& operator[](T&& key)
            requires std::is_same_v<std::decay_t<T>, std::string> ||
                     std::is_constructible_v<std::string, std::decay_t<T>>
        {
            return std::get<object>(m_value)[std::forward<T>(key)];
        }

        const value& operator[](const std::string& key) const
        {
            return at(key);
        }

        value& at(const std::string& key)
        {
            return std::get<object>(m_value).at(key);
        }

        const value& at(const std::string& key) const
        {
            return std::get<object>(m_value).at(key);
        }

        // Array elements access
        value& operator[](std::size_t index)
        {
            return std::get<array>(m_value)[index];
        }

        const value& operator[](std::size_t index) const
        {
            return std::get<array>(m_value)[index];
        }

        value& at(std::size_t index)
        {
            return std::get<array>(m_value).at(index);
        }

        const value& at(std::size_t index) const
        {
            return std::get<array>(m_value).at(index);
        }

        // Getter function, basically wrapper around std::get()
        template <typename T>
        T get() const
            requires std::is_same_v<uint64_t, T> || std::is_same_v<int64_t, T> || std::is_same_v<double, T> ||
                     std::is_same_v<bool, T> || std::is_same_v<std::nullptr_t, T>
        {
            using enum constant;

            if constexpr (std::is_same_v<bool, T>)
            {
                const auto v = std::get<constant>(m_value);

                return v == JSON_NULL ? throw std::bad_variant_access() : v == JSON_TRUE;
            }
            else if constexpr (std::is_same_v<std::nullptr_t, T>)
            {
                return std::get<constant>(m_value) == JSON_NULL ? nullptr : throw std::bad_variant_access();
            }
            else
            {
                return std::get<T>(m_value);
            }
        }

        template <typename T>
        T& get()
            requires std::is_same_v<object, T> || std::is_same_v<array, T> || std::is_same_v<std::string, T>
        {
            return std::get<T>(m_value);
        }

        template <typename T>
        const T& get() const
            requires std::is_same_v<object, T> || std::is_same_v<array, T> || std::is_same_v<std::string, T>
        {
            return std::get<T>(m_value);
        }

        void set(const value& other, const bool same_type_check = true)
        {
            if (same_type_check && !has_same_type(other))
            {
                throw incompatible_assignment_exception(
                    MAKE_ERROR_MSG("Attempt to set new value has failed due to incompatible types, current type: ",
                        get_type_string(*this), ", new type: ", get_type_string(other)));
            }

            m_value = other.m_value;
        }

        void set(value&& other, const bool same_type_check = true)
        {
            if (same_type_check && !has_same_type(other))
            {
                throw incompatible_assignment_exception(
                    MAKE_ERROR_MSG("Attempt to set new value has failed due to incompatible types, current type: ",
                        get_type_string(*this), ", new type: ", get_type_string(other)));
            }

            m_value = std::move(other.m_value);
        }

        template <typename... Ts>
        static constexpr array make_array(Ts&&... ts)
        {
            array arr;

            if constexpr (sizeof...(Ts) > 0)
            {
                arr.reserve(sizeof...(Ts));

                (arr.emplace_back(value(std::forward<Ts>(ts))), ...);
            }

            return arr;
        }

        static object make_object()
        {
            return object();
        }

    private:

        template <typename K, typename V, typename... Ts>
        static constexpr object make_object(K&& key, V&& val, Ts&&... ts) 
            requires requires(K, Ts...) {
                requires std::is_constructible_v<std::string, std::decay_t<K> >;
                requires sizeof...(Ts) % 2 == 0;
            }
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
                throw duplicate_key_exception(
                    MAKE_ERROR_MSG("Duplicate key '", key, "' while trying to create json object"));
            }

            if constexpr (sizeof...(Ts) > 0)
            {
                make_object(obj, std::forward<Ts>(ts)...);
            }
        }

        static std::string_view get_type_string(const value& v)
        {
            static std::array<std::string_view, std::variant_size_v<internal_value_type>> type_names =
            {
                "object",
                "array",
                "string",
                "unsigned int",
                "signed int",
                "double",
                "boolean"
            };

            return v.is_null() ? "null" : type_names[v.m_value.index()];
        }
    };
}