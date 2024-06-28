#ifndef HAL_x86_64_APIC_HPP
#define HAL_x86_64_APIC_HPP

#include <library/common/types.hpp>

namespace a9n::hal::x86_64
{
    namespace LOCAL_APIC_ADDRESS
    {
        constexpr static uint32_t BASE_REGISTER          = 0xfee00000;
        constexpr static uint32_t ID_REGISTER            = BASE_REGISTER + 0x20;
        constexpr static uint32_t VERSION                = BASE_REGISTER + 0x30;
        constexpr static uint32_t TASK_PRIORITY_REGISTOR = BASE_REGISTER + 0x80;
        constexpr static uint32_t ARBITRATION_PRIORITY_REGISTOR
            = BASE_REGISTER + 0x90;
        constexpr static uint32_t PROCESSOR_PRIORITY_REGISTOR
            = BASE_REGISTER + 0xa0;
        constexpr static uint32_t EOI = BASE_REGISTER + 0xb0;
    }

    namespace IO_APIC_ADDRESS
    {
        constexpr static uint32_t IO_REGISTER_SELECT = 0xfec00000;
        constexpr static uint32_t IO_REGISTER_WINDOW = 0xfec00010;
    }

}

#endif
