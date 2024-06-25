#pragma once

#include <iomanip>
#include <ostream>
#include <string>

#include "value.h"
#include "value_visitor.h"


namespace tdg::json
{
	template <
		std::ios_base& float_format(std::ios_base&) = std::fixed,
		std::size_t precision = 6u,
		std::size_t tab_width = 3u
		>
	class pretty_printer : public value_visitor
	{
		static constexpr std::size_t INITIAL_FILLER_STRING_SIZE = 16u * tab_width;

	public:
		explicit pretty_printer(std::ostream& oss) : m_out(oss) {}

		void print(const value& start_value)
		{
			auto current_precision = m_out.precision();

			if (current_precision != precision)
			{
				m_out << std::setprecision(precision);
			}

			start_value.accept(*this);

			if (current_precision != precision)
			{
				m_out << std::setprecision(current_precision);
			}
		}

		void visit(const std::string& s) const override
		{
			m_out << std::quoted(s);
		}

		void visit(int64_t sint) const override
		{
			m_out << sint;
		}

		void visit(uint64_t uint) const override
		{
			m_out << uint;
		}

		void visit(double d) const override
		{
			m_out << float_format << d;
		}

		void visit(bool b) const override
		{
			m_out << (b ? "true" : "false");
		}

		void visit(nullptr_t) const override
		{
			m_out << "null";
		}

		void visit(const array& values) const override
		{
			m_out << "[\n";
			push_level();

			for (auto seen_first = false; const auto & value : values)
			{
				if (seen_first)
				{
					m_out << ",\n";
				}

				m_out << indent();

				value.accept(*this);

				seen_first = true;
			}

			pop_level();
			m_out << '\n' << indent() << ']';
		}

		void visit(const object& obj) const override
		{
			m_out << "{\n";
			push_level();

			for (auto seen_first = false; const auto & [key, val] : obj)
			{
				if (seen_first)
				{
					m_out << ",\n";
				}

				m_out << indent() << std::quoted(key) << " : ";
				val.accept(*this);

				seen_first = true;
			}

			pop_level();
			m_out << '\n' << indent() << '}';
		}

	private:
		void push_level() const { ++m_level; }
		void pop_level() const { --m_level; }

		std::string_view indent() const
		{
			const auto required_size = m_level * tab_width;

			if (m_filler_string.capacity() < required_size)
			{
				m_filler_string.resize(2u * (m_level - 1) * tab_width, ' ');
			}

			return std::string_view(m_filler_string.data(), required_size);
		}

		std::ostream& m_out;
		mutable std::size_t m_level = 0u;
		mutable std::string m_filler_string = std::string(INITIAL_FILLER_STRING_SIZE, ' ');
	};

	template <
		std::ios_base& float_format(std::ios_base&),
		std::size_t precision,
		std::size_t tab_width
		>
	constexpr std::size_t INITIAL_FILLER_STRING_SIZE;
}
