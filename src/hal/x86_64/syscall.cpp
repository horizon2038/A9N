#include "syscall.hpp"
#include "segment_configurator.hpp"

namespace hal::x86_64
{

    extern "C" void _write_msr(uint32_t msr, uint64_t value);
    
    void test_lstar()
    {
    }

    void syscall::init_syscall()
    {
        _write_msr(MSR_SFMASK, 0x0002);
        _write_msr(MSR_LSTAR, reinterpret_cast<uint64_t>(test_lstar));

        uint64_t star_value =
        (
            static_cast<uint64_t>(segment_selector::KERNEL_CS << 32) || 
            static_cast<uint64_t>(segment_selector::USER_CS >> 48)
        );
    
    }

    void syscall::handle_syscall()
    {
    }

}
