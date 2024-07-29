#ifndef A9N_KERNEL_IPC_BUFFER_HPP
#define A9N_KERNEL_IPC_BUFFER_HPP

#include <kernel/types.hpp>
#include <liba9n/libcxx/array>

namespace a9n::kernel
{
    // calculate buffer length

    inline consteval a9n::word calculate_buffer_length()
    {
        // reserved
        // - destination_descriptor (sizeof(capability_descriptor))
        // - destination_depth      (sizeof(word))
        // - message_tag            (sizeof(message_tag))
        constexpr a9n::word max_length = a9n::PAGE_SIZE / a9n::WORD_BITS;
        constexpr a9n::word reserved   = sizeof(a9n::word) * 3;
        return max_length - reserved;
    }

    inline constexpr a9n::word MESSAGE_BUFFER_SIZE_MAX = calculate_buffer_length();

    using message_buffer_array
        = liba9n::std::array<a9n::word, MESSAGE_BUFFER_SIZE_MAX>;

    struct ipc_buffer
    {
        a9n::capability_descriptor destination_descriptor;
        a9n::word                  destination_depth;
        a9n::word                  message_tag;
        message_buffer_array       messages;

        // similar to std::get<auto Index>()
        // compile-time boundary check
        template<a9n::word Index>
            requires(Index < MESSAGE_BUFFER_SIZE_MAX)
        constexpr a9n::word get_message()
        {
            return messages[Index];
        }

        template<a9n::word Index>
            requires(Index < MESSAGE_BUFFER_SIZE_MAX)
        constexpr a9n::word get_message() const
        {
            return messages[Index];
        }

        constexpr a9n::word get_message(a9n::word index)
        {
            return messages[index];
        }

        constexpr a9n::word get_message(a9n::word index) const
        {
            return messages[index];
        };

        template<a9n::word Index>
            requires(Index < MESSAGE_BUFFER_SIZE_MAX)
        constexpr void set_message(a9n::word value)
        {
            messages[Index] = value;
        }

        constexpr void set_message(a9n::word index, a9n::word value)
        {
            messages[index] = value;
        }
    };

}

#endif
