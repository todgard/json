#pragma once

#include <iomanip>
#include <ostream>

#include "value.h"
#include "value_visitor.h"


namespace tdg::json
{
	template <
		std::ios_base& float_format(std::ios_base&) = std::fixed,
		std::size_t precision = 6u
		>
	class printer : public value_visitor
	{
	public:
		explicit printer(std::ostream& oss) : m_out(oss) {}

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

		virtual void visit(const std::string& s) const
		{
			m_out << std::quoted(s);
		}

		virtual void visit(int64_t sint) const
		{
			m_out << sint;
		}

		virtual void visit(uint64_t uint) const
		{
			m_out << uint;
		}

		virtual void visit(double d) const
		{
			m_out << float_format << d;
		}
		
		virtual void visit(bool b) const
		{
			m_out << (b ? "true" : "false");
		}

		virtual void visit(nullptr_t) const
		{
			m_out << "null";
		}

		virtual void visit(const array& items) const
		{
			m_out << '[';

			for (auto seen_first = false; const auto& i : items)
			{
				if (seen_first)
				{
					m_out << ',';
				}

				i.accept(*this);

				seen_first = true;
			}

			m_out << ']';
		}

		virtual void visit(const object& obj) const
		{
			m_out << '{';

			for (auto seen_first = false; const auto& [key, val] : obj)
			{
				if (seen_first)
				{
					m_out << ',';
				}

				m_out << std::quoted(key) << ':';
				val.accept(*this);

				seen_first = true;
			}

			m_out << '}';
		}

	private:
		std::ostream& m_out;
	};
}
