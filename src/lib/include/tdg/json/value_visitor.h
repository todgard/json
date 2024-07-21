#pragma once

#include <string>

#include "tdg/json/jsonfwd.h"


namespace tdg::json
{
    class value_visitor
    {
    public:
        virtual void visit(const std::string&) = 0;
        virtual void visit(int64_t) = 0;
        virtual void visit(uint64_t) = 0;
        virtual void visit(double) = 0;
        virtual void visit(bool) = 0;
        virtual void visit(nullptr_t) = 0;
        virtual void visit(const array&) = 0;
        virtual void visit(const object&) = 0;
        virtual ~value_visitor() = default;
    };
}
