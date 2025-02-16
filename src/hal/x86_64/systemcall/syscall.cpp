#include "hal/x86_64/interrupt/interrupt.hpp"
#include <hal/x86_64/systemcall/syscall.hpp>

#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/arch/rflags.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>

#include <kernel/kernelcall/kernel_call.hpp>
#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    extern "C" void _syscall_handler(void);

    hal_result init_syscall()
    {
        // enable syscall
        _write_msr(msr::EFER, _read_msr(msr::EFER) | efer_flag::SYSCALL_EXTENSION | efer_flag::NO_EXECUTE);
        // mask rflags
        _write_msr(msr::SFMASK, rflags_flag::INTERRUPT);
        // setup syscall entry point
        _write_msr(msr::LSTAR, reinterpret_cast<uint64_t>(_syscall_handler));
        a9n::kernel::utility::logger::printh(
            "_syscall_handler address : 0x%016llx\n",
            reinterpret_cast<uint64_t>(_syscall_handler)
        );

        // When a `syscall` occurs in Long Mode, the segments are loaded as
        // follows:
        // 1. The Kernel CS is loaded from the MSR STAR[47-32].
        // 2. The Kernel SS is loaded from CS + 0x08 obtained in step 1.
        // Therefore, the Kernel Code Segment and Data Segment are naturally
        // placed in sequence.
        //
        // The issue arises during `sysret`, where the segments are loaded as
        // follows:
        // 1. The User CS is loaded from MSR STAR[63-48] + 0x10.
        // 2. The User SS is loaded as CS - 0x08 from step 1.
        //
        // This is an odd specification. Therefore, it's necessary to specify a
        // Null Selector in STAR[63-48], followed by the User Data Segment, and
        // then the User Code Segment.
        uint64_t star_value = (static_cast<uint64_t>(segment_selector::USER_CS_NULL) << 48)
                            | (static_cast<uint64_t>(segment_selector::KERNEL_CS) << 32);
        a9n::kernel::utility::logger::printh("STAR value : 0x%016llx\n", star_value);
        _write_msr(msr::STAR, star_value);
        _write_msr(msr::CSTAR, 0);

        return {};
    }

    extern "C" void do_syscall(a9n::sword kernel_call_number)
    {
        [[unlikely]] if (!kernel_call_handler)
        {
            a9n::kernel::utility::logger::printh(
                "kernel call handler "
                "is not set\n"
            );

            return;
        }

        auto type = static_cast<a9n::kernel::kernel_call_type>(kernel_call_number);
        switch (type)
        {
            using enum a9n::kernel::kernel_call_type;

            case CAPABILITY_CALL :
                [[fallthrough]];
            case YIELD :
                [[fallthrough]];
            case DEBUG :
                kernel_call_handler(type);
                return;

            [[unlikely]] default :
                fault_dispatcher(a9n::kernel::fault_type::INVALID_KERNEL_CALL, kernel_call_number, 0);
        }
    }

}
