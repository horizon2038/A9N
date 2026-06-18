#ifndef A9N_KERNEL_INIT_HPP
#define A9N_KERNEL_INIT_HPP

#include <kernel/boot/boot_info.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/types.hpp>

#define KERNEL_MAJOR_VERSION 0
#define KERNEL_MINOR_VERSION 1
#define KERNEL_PATCH_VERSION 0

namespace a9n::kernel
{
    inline constexpr a9n::word INITIAL_PROCESS_ROOT_NODE_COUNT  = 256;
    inline constexpr a9n::word INITIAL_PROCESS_ROOT_NODE_RADIX  = 8;

    inline constexpr a9n::word INITIAL_FRAME_COUNT_MAX          = 4096 * 8;
    inline constexpr a9n::word INITIAL_PAGE_TABLE_COUNT_MAX     = 128;

    inline constexpr a9n::word INITIAL_GENERIC_COUNT_MAX        = 128;
    inline constexpr a9n::word INITIAL_INTERRUPT_PORT_COUNT_MAX = 128;

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
        PROCESS_ADDRESS_SPACE,
        PROCESS_ROOT_NODE, // *recursive*
        PROCESS_PAGE_TABLE_NODE,
        PROCESS_FRAME_NODE,
        PROCESS_IPC_BUFFER_FRAME,
        GENERIC_NODE, // initial generics
        INTERRUPT_REGION,
        IO_PORT,
    };

    struct init_info
    {
        // kernel description
        a9n::word kernel_major_version {};
        a9n::word kernel_minor_version {};
        a9n::word kernel_patch_version {};
        char      kernel_pre_release[32] {};
        char      kernel_build_meta_data[32] {};

        // architectural information
        a9n::word arch_info[ARCH_INFO_MAX];

        // initial ipc buffer
        a9n::virtual_address ipc_buffer;

        // initial generic
        generic_descriptor generic_list[INITIAL_GENERIC_COUNT_MAX];
        a9n::word          generic_list_count;
    };

    static_assert(
        sizeof(init_info) <= a9n::PAGE_SIZE,
        "init_info must be less than or equal to the page size"
    );

    kernel_result create_init(const boot_info &info);
}

#endif
