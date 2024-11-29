#ifndef A9N_KERNEL_INIT_HPP
#define A9N_KERNEL_INIT_HPP

#include "kernel/types.hpp"
#include <kernel/boot/boot_info.hpp>
#include <kernel/kernel_result.hpp>

namespace a9n::kernel
{
    inline constexpr a9n::word INITIAL_FRAME_COUNT_MAX      = 256;
    inline constexpr a9n::word INITIAL_PAGE_TABLE_COUNT_MAX = 256;

    inline constexpr a9n::word INITIAL_GENERIC_COUNT_MAX    = 256;
    inline constexpr a9n::word INITIAL_IPC_PORT_COUNT_MAX   = 256;

    struct generic_descriptor
    {
        a9n::physical_address address;
        uint8_t               size_radix;
        bool                  is_device;
    };

    struct interrupt_port_descriptor
    {
        a9n::word irq_number;
    };

    enum class init_slot_offset : a9n::word
    {
        RESERVED,
        PROCESS_CONTROL_BLOCK,
        PROCESS_ROOT_PAGE_TABLE,
        PROCESS_ROOT_NODE, // *recursive*
        PROCESS_PAGE_TABLE_NODE,
        PROCESS_FRAME_NODE,
        GENERIC_NODE, // initial generics
    };

    struct init_info
    {
        // kernel description
        a9n::word kernel_version {};

        // initial ipc buffer
        a9n::virtual_address       ipc_buffer;
        a9n::capability_descriptor frame_ipc_buffer;

        // initial generic
        generic_descriptor         generic_list[INITIAL_GENERIC_COUNT_MAX];
        a9n::capability_descriptor generic_start;
        a9n::word                  generic_list_count;

        // initial interrupt ports
        interrupt_port_descriptor  interrupt_port_list[INITIAL_IPC_PORT_COUNT_MAX];
        a9n::capability_descriptor interrupt_port_start;
        a9n::word                  interrupt_port_count;
    };

    static_assert(sizeof(init_info) <= a9n::PAGE_SIZE * 2);

    kernel_result create_init(const boot_info &info);
}

#endif
