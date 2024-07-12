#pragma once

#include <functional>
#include <type_traits>

#include "basic_macros.h"


namespace tdg::util
{
    class on_scope_exit final
    {
    public:
        template <typename T>
        explicit on_scope_exit(T p) requires std::is_invocable_r_v<void, T>
            : m_callback(p)
        {
        }

        on_scope_exit(const on_scope_exit&) = delete;
        on_scope_exit& operator=(const on_scope_exit&) = delete;

        ~on_scope_exit()
        {
            if (m_callback)
            {
                m_callback();
            }
        }

        void cancel()
        {
            m_callback = nullptr;
        }

    private:
        std::function<void()> m_callback;
    };
}

#define ON_SCOPE_EXIT(body) \
auto MAKE_UNIQUE_VAR(scope_exit_guard_) = tdg::util::on_scope_exit([&]() { body; })

