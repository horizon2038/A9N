#ifndef A9N_HAL_X86_64_ARCH_CONTROL_REGISTER_HPP
#define A9N_HAL_X86_64_ARCH_CONTROL_REGISTER_HPP

#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    extern "C" void     _write_cr0(uint64_t data);
    extern "C" uint64_t _read_cr0(void);

    extern "C" void     _write_cr2(uint64_t data);
    extern "C" uint64_t _read_cr2(void);

    extern "C" void     _write_cr3(uint64_t data);
    extern "C" uint64_t _read_cr3(void);

    extern "C" void     _write_cr4(uint64_t data);
    extern "C" uint64_t _read_cr4(void);

    namespace cr0_flag
    {
        inline constexpr uint64_t PROTECTED_MODE      = 1 << 0;
        inline constexpr uint64_t MONITOR_COPROCESSOR = 1 << 1;
        inline constexpr uint64_t EMULATION           = 1 << 2;
        inline constexpr uint64_t TASK_SWITCHED       = 1 << 3;
        inline constexpr uint64_t EXTENSION_TYPE      = 1 << 4;
        inline constexpr uint64_t NUMERIC_ERROR       = 1 << 5;
        inline constexpr uint64_t WRITE_PROTECT       = 1 << 16;
        inline constexpr uint64_t ALIGNMENT_MASK      = 1 << 18;
        inline constexpr uint64_t NOT_WRITE_THROUGH   = 1 << 29;
        inline constexpr uint64_t CACHE_DISABLE       = 1 << 30;
        inline constexpr uint64_t PAGING              = 1 << 31;
    }

    namespace cr4_flag
    {
        inline constexpr uint64_t VIRTUAL_8086_MODE_EXTENSION         = 1 << 0;
        inline constexpr uint64_t PROTECTED_MODE_VIRTUAL_INTERRUPT    = 1 << 1;
        inline constexpr uint64_t TIME_STAMP                          = 1 << 2;
        inline constexpr uint64_t DEBUGGING_EXTENSION                 = 1 << 3;
        inline constexpr uint64_t PAGE_SIZE_EXTENSION                 = 1 << 4;
        inline constexpr uint64_t PHYSICAL_ADDRESS_EXTENSION          = 1 << 5;
        inline constexpr uint64_t MACHINE_CHECK_EXCEPTION             = 1 << 6;
        inline constexpr uint64_t PAGE_GLOBAL                         = 1 << 7;
        inline constexpr uint64_t PERFORMANCE_COUNTER                 = 1 << 8;
        inline constexpr uint64_t OS_FX_SAVE_RESTORE                  = 1 << 9;
        inline constexpr uint64_t OS_XMM_EXCEPTION                    = 1 << 10;
        inline constexpr uint64_t USER_MODE_INSTRUCTION_PREVENTION    = 1 << 11;
        inline constexpr uint64_t VIRTUAL_MACHINE_EXTENSION           = 1 << 13;
        inline constexpr uint64_t SAFER_MODE_EXTENSION                = 1 << 14;
        inline constexpr uint64_t FS_GS_BASE                          = 1 << 16;
        inline constexpr uint64_t PCID                                = 1 << 17;
        inline constexpr uint64_t OS_X_SAVE                           = 1 << 18;
        inline constexpr uint64_t SV_MODE_EXECUTION_PROTECTION        = 1 << 20;
        inline constexpr uint64_t SV_MODE_ACCESS_PROTECTION           = 1 << 21;
        inline constexpr uint64_t PROTECTION_KEY_USER                 = 1 << 22;
        inline constexpr uint64_t CONTROL_FLOW_ENFORCEMENT_TECHNOLOGY = 1 << 23;
        inline constexpr uint64_t PROTECTION_KEYS_SV                  = 1 << 24;
    }
}

#endif
