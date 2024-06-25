#pragma once

#include <string>

#include "jsonfwd.h"


namespace tdg::json
{
	class value_visitor
	{
	public:
		virtual void visit(const std::string&) const = 0;
		virtual void visit(int64_t) const = 0;
		virtual void visit(uint64_t) const = 0;
		virtual void visit(double) const = 0;
		virtual void visit(bool) const = 0;
		virtual void visit(nullptr_t) const = 0;
		virtual void visit(const array&) const = 0;
		virtual void visit(const object&) const = 0;
		virtual ~value_visitor() = default;
	};
}
