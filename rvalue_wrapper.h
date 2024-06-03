#pragma once

namespace tdg
{
	template <typename T>
	struct rvalue_wrapper
	{
		rvalue_wrapper() = delete;
		rvalue_wrapper(const T&) = delete;
		void* operator new(std::size_t) = delete;

		rvalue_wrapper(T&& value) : m_value(std::move(value))
		{}

		T&& get() const
		{
			return std::move(m_value);
		}

		T& m_value;
	};
}
