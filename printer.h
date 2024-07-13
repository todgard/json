#pragma once

#include <iomanip>
#include <ostream>

#include "value.h"
#include "util.h"
#include "value_visitor.h"


namespace tdg::json
{
	//TODO: make it non-movable and non-copyable, make ctor protected so it cannot be created on heap
	//consider also hiding this class by using stream wrapper and operator <<, so the printing operation
	//is as simple as for example: std::cout << value;

	template <
		std::ios_base& float_format(std::ios_base&) = std::fixed,
		std::size_t precision = 6u
		>
	class printer : public value_visitor
	{
	public:
		explicit printer(std::ostream& oss) : m_out(oss) {}

		void print(const value& val)
		{
			auto current_precision = m_out.precision();

            m_out << std::setprecision(precision);

			ON_SCOPE_EXIT(m_out << std::setprecision(current_precision));

			val.accept(*this);
		}

		void visit(const std::string& s) const override
		{
			m_out << '"' << s << '"';
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

		void visit(const array& items) const override
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

		void visit(const object& obj) const override
		{
			m_out << '{';

			for (auto seen_first = false; const auto& [key, val] : obj)
			{
				if (seen_first)
				{
					m_out << ',';
				}

				m_out << '"' << key << '"' << ':';
				val.accept(*this);

				seen_first = true;
			}

			m_out << '}';
		}

	protected:
		std::ostream& m_out;
	};
}
