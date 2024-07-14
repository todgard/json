#pragma once

#include <iomanip>
#include <ostream>
#include <string>

#include "tdg/json/printer.h"


namespace tdg::json
{
	template <
		std::ios_base& float_format(std::ios_base&) = std::fixed,
		std::size_t precision = 6u,
		std::size_t tab_width = 3u
		>
	class pretty_printer : public printer<float_format, precision>
	{
		static constexpr std::size_t INITIAL_FILLER_STRING_SIZE = 16u * tab_width;

		using base_t = printer<float_format, precision>;

	public:
		explicit pretty_printer(std::ostream& oss) : base_t(oss) {}

		void visit(const array& values) const override
		{
			if (values.empty())
			{
				this->m_out << "[]";
				return;
			}

			this->m_out << "[\n";
			push_level();

			for (auto seen_first = false; const auto & value : values)
			{
				if (seen_first)
				{
					this->m_out << ",\n";
				}

				this->m_out << indent();

				value.accept(*this);

				seen_first = true;
			}

			pop_level();
			this->m_out << '\n' << indent() << ']';
		}

		void visit(const object& obj) const override
		{
			if (obj.empty())
			{
				this->m_out << "{}";
				return;
			}

			this->m_out << "{\n";
			push_level();

			for (auto seen_first = false; const auto & [key, val] : obj)
			{
				if (seen_first)
				{
					this->m_out << ",\n";
				}

				this->m_out << indent() << '"' << key  << "\" : ";
				val.accept(*this);

				seen_first = true;
			}

			pop_level();
			this->m_out << '\n' << indent() << '}';
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

