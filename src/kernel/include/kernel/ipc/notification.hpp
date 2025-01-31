#ifndef A9N_KERNEL_NOTIFICATION_HPP
#define A9N_KERNEL_NOTIFICATION_HPP

#include <kernel/types.hpp>

namespace a9n::kernel
{
    struct notification
    {
        enum class state : a9n::word
        {
            NONE,
            HAS_FLAG,
        } current_state { state::NONE };
        a9n::word flag { 0 };

        constexpr bool has_notification(void) const
        {
            return (current_state == state::HAS_FLAG) && (flag != 0);
        }

        constexpr void notify(a9n::word new_flag)
        {
            flag          |= new_flag;
            current_state  = state::HAS_FLAG;
        }

        constexpr a9n::word consume(void)
        {
            auto temp_flag = flag;
            flag           = 0;
            current_state  = state::NONE;
            return temp_flag;
        }
    };
}

#endif
