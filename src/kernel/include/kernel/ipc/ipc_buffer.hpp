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
        // - transfer information
        constexpr a9n::word max_length = a9n::PAGE_SIZE / sizeof(a9n::word); // unit: a9n::word
        constexpr a9n::word reserved   = 16 + 2; // 16 for transfer, 2 for
                                                 // destination (node descriptor
                                                 // + index)
        return (max_length - reserved);
    }

    inline constexpr a9n::word MESSAGE_BUFFER_SIZE_MAX       = calculate_buffer_length();
    inline constexpr a9n::word CAPABILITY_TRANSFER_COUNT_MAX = 16;

    using message_buffer_array = liba9n::std::array<a9n::word, MESSAGE_BUFFER_SIZE_MAX>;

    struct ipc_buffer
    {
        message_buffer_array messages;

        // for capability transfer
        // send
        liba9n::std::array<capability_descriptor, CAPABILITY_TRANSFER_COUNT_MAX> transfer_source_descriptors;
        // receive
        a9n::capability_descriptor transfer_destination_node;
        a9n::word                  transfer_destination_index;

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

    static_assert(sizeof(ipc_buffer) <= a9n::PAGE_SIZE);

}

#endif
