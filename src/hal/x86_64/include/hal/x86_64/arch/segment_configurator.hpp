#ifndef GDT_INITIALIZER_HPP
#define GDT_INITIALIZER_HPP

#include <stdint.h>

namespace hal::x86_64
{
    namespace segment_selector
    {
        constexpr static uint16_t NULL_SEGMENT = 0x00;
        constexpr static uint16_t KERNEL_CS = 0x08;
        constexpr static uint16_t KERNEL_DS = 0x10;
        constexpr static uint16_t USER_CS_NULL = 0x18;
        constexpr static uint16_t USER_CS = 0x20;
        constexpr static uint16_t USER_DS = 0x28;
        constexpr static uint16_t KERNEL_TSS = 0x30;
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
    }__attribute__((packed));

    struct global_descriptor_table
    {
        uint64_t null;
        uint64_t kernel_code_segment;
        uint64_t kernel_data_segment;
        uint64_t user_code_segment_32; // unused
        uint64_t user_code_segment;
        uint64_t user_data_sgment;
        uint64_t tss_low;
        uint64_t tss_high;

    }__attribute__((packed));

    class segment_configurator
    {
        public:
            segment_configurator();
            ~segment_configurator();

            void init_gdt();
            void configure_rsp0(uint64_t kernel_stack_pointer);

        private:
            void configure_gdt();
            void configure_tss();

            void load_gdt();
            void load_segment_register(uint16_t code_segment_register);
            void load_task_register(uint16_t segment_register);

            // it needs to be rewritten because it does not take into account mp
            static task_state_segment tss;
            static global_descriptor_table gdt;

    };

}
#endif
