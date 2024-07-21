#pragma once

#include <functional>
#include <type_traits>

#include "tdg/util/basic_macros.h"


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

#define ON_SCOPE_EXIT_CAP1(cap1, body) \
auto MAKE_UNIQUE_VAR(scope_exit_guard_) = tdg::util::on_scope_exit([cap1]() { body; })

#define ON_SCOPE_EXIT_CAP2(cap1, cap2, body) \
auto MAKE_UNIQUE_VAR(scope_exit_guard_) = tdg::util::on_scope_exit([cap1, cap2]() { body; })

#define ON_SCOPE_EXIT_CAP3(cap1, cap2, cap3, body) \
auto MAKE_UNIQUE_VAR(scope_exit_guard_) = tdg::util::on_scope_exit([cap1, cap2, cap3]() { body; })

#define ON_SCOPE_EXIT_CAP4(cap1, cap2, cap3, cap4, body) \
auto MAKE_UNIQUE_VAR(scope_exit_guard_) = tdg::util::on_scope_exit([cap1, cap2, cap3, cap4]() { body; })

#define ON_SCOPE_EXIT_CAP5(cap1, cap2, cap3, cap4, cap5, body) \
auto MAKE_UNIQUE_VAR(scope_exit_guard_) = tdg::util::on_scope_exit([cap1, cap2, cap3, cap4, cap5]() { body; })
