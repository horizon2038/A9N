#ifndef GDT_INITIALIZER_HPP
#define GDT_INITIALIZER_HPP

#include "hal/x86_64/interrupt/interrupt_descriptor.hpp"
#include <hal/hal_result.hpp>
#include <stdint.h>

namespace a9n::hal::x86_64
{
    namespace segment_selector
    {
        static constexpr uint16_t NULL_SEGMENT = 0x00;
        static constexpr uint16_t KERNEL_CS    = 0x08;
        static constexpr uint16_t KERNEL_DS    = 0x10;
        static constexpr uint16_t USER_CS_NULL = 0x18;
        static constexpr uint16_t USER_DS      = 0x20;
        static constexpr uint16_t USER_CS      = 0x28;
        static constexpr uint16_t KERNEL_TSS   = 0x30;
    };

    struct task_state_segment
    {
        uint32_t reserved_0;
        uint64_t rsp_0;
        uint64_t rsp_1;
        uint64_t rsp_2;
        uint64_t reserved_1;
        uint64_t ist_1;
        uint64_t ist_2;
        uint64_t ist_3;
        uint64_t ist_4;
        uint64_t ist_5;
        uint64_t ist_6;
        uint64_t ist_7;
        uint64_t reserved_2;
        uint16_t reserved_3;
        uint16_t iobp_offset;
    } __attribute__((packed));

    struct global_descriptor_table
    {
        uint64_t null;
        uint64_t kernel_code_segment;
        uint64_t kernel_data_segment;
        uint64_t user_code_segment_32; // unused
        uint64_t user_data_segment;
        uint64_t user_code_segment;
        uint64_t tss_low;
        uint64_t tss_high;

    } __attribute__((packed));

    namespace segment
    {
        hal_result init();
        void       configure_gdt(global_descriptor_table &gdt);
        void       configure_idt(interrupt_descriptor_table &idt);
        void       configure_tss(global_descriptor_table &gdt, task_state_segment &tss);

        void load_gdt(global_descriptor_table &gdt);
        void load_segment_register(uint16_t code_segment_register);
        void load_task_register(uint16_t segment_register);
        void load_idt(interrupt_descriptor_table &idt);
    }

    /*
    class segment_configurator
    {
      public:
        hal_result init();
        void       configure_rsp0(uint64_t kernel_stack_pointer);

      private:
        void configure_gdt();
        void configure_tss();

        void load_gdt();
        void load_segment_register(uint16_t code_segment_register);
        void load_task_register(uint16_t segment_register);
    };
    */

    inline constexpr uint64_t
        create_segment_descriptor(uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
    {
        // +------------+-------+-------+-------------+-----------+
        // |   63-56    | 55-52 | 51-48 |    47-40    |   39-32   |
        // +------------+-------+-------+-------------+-----------+
        // | base(high) | flags | limit | access byte | base(mid) |
        // +------------+-------+-------+-------------+-----------+
        // |           31-16            |          15-0           |
        // +----------------------------+-------------------------+
        // | base(low)                  | limit                   |
        // +----------------------------+-------------------------+

        uint64_t descriptor {};

        descriptor |= static_cast<uint64_t>((base >> 24) & 0xFF) << 56;
        descriptor |= static_cast<uint64_t>(flags & 0xF) << 52;
        descriptor |= static_cast<uint64_t>((limit >> 16) & 0xF) << 48;
        descriptor |= static_cast<uint64_t>(access) << 40;
        descriptor |= static_cast<uint64_t>((base >> 16) & 0xFF) << 32;
        descriptor |= static_cast<uint64_t>(base & 0xFFFF) << 16;
        descriptor |= static_cast<uint64_t>(limit & 0xFFFF);

        return descriptor;
    }

    inline constexpr uint8_t create_segment_access_field(
        bool is_present,
        bool is_user,
        bool is_system_segment,
        bool is_executable,
        bool is_comforming,
        bool is_readable_writable,
        bool is_accessed = true
    )
    {
        // +---------+-----+---+---+----+----+---+
        // |    7    | 6-5 | 4 | 3 | 2  | 1  | 0 |
        // +---------+-----+---+---+----+----+---+
        // | present | dpl | s | e | dc | rw | a |
        // +---------+-----+---+---+----+----+---+

        uint8_t descriptor {};

        descriptor |= static_cast<uint8_t>(is_present << 7);
        descriptor |= static_cast<uint8_t>((is_user ? 3 : 0) << 5);
        descriptor |= static_cast<uint8_t>(!is_system_segment << 4);
        descriptor |= static_cast<uint8_t>(is_executable << 3);
        descriptor |= static_cast<uint8_t>(is_comforming << 2);
        descriptor |= static_cast<uint8_t>(is_readable_writable << 1);
        descriptor |= static_cast<uint8_t>(is_accessed << 0);

        return descriptor;
    }

    namespace system_segment_type
    {
        inline constexpr uint8_t LDT              = 0x2;
        inline constexpr uint8_t TSS_AVAILABLE_64 = 0x9;
        inline constexpr uint8_t TSS_WAIT_64      = 0xB;
    };

    inline constexpr uint8_t
        create_system_segment_access_field(bool is_present, bool is_user, uint8_t type)
    {
        uint8_t descriptor {};

        descriptor |= static_cast<uint8_t>(is_present << 7);
        descriptor |= static_cast<uint8_t>((is_user ? 3 : 0) << 5);
        descriptor |= static_cast<uint8_t>(type & 0xF);

        return descriptor;
    }

    namespace segment_descriptor
    {
        // in long mode, the base and limit are ignored.
        namespace detail
        {
            inline constexpr uint32_t BASE  = 0;
            inline constexpr uint32_t LIMIT = 0xFFFFF;
            inline constexpr uint8_t  CODE  = 0xA; // limit 4kib, long mode
            inline constexpr uint8_t  DATA  = 0xA; // limit 4kib, long mode
        }

        inline constexpr uint64_t KERNEL_CODE = create_segment_descriptor(
            detail::BASE,
            detail::LIMIT,
            create_segment_access_field(true, false, false, true, false, true),
            detail::CODE
        );

        inline constexpr uint64_t KERNEL_DATA = create_segment_descriptor(
            detail::BASE,
            detail::LIMIT,
            create_segment_access_field(true, false, false, false, false, true),
            detail::DATA
        );

        inline constexpr uint64_t USER_CODE = create_segment_descriptor(
            detail::BASE,
            detail::LIMIT,
            create_segment_access_field(true, true, false, true, false, true),
            detail::CODE
        );

        inline constexpr uint64_t USER_DATA = create_segment_descriptor(
            detail::BASE,
            detail::LIMIT,
            create_segment_access_field(true, true, false, false, false, true),
            detail::DATA
        );
    }

    // TODO: move to cpu_local_variable
    /*
    alignas(8) inline task_state_segment tss;
    alignas(8) inline global_descriptor_table gdt;
    */
}
#endif
