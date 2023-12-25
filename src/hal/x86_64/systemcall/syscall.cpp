#include <hal/x86_64/systemcall/syscall.hpp>

#include <hal/x86_64/arch/segment_configurator.hpp>

namespace hal::x86_64
{

    extern "C" void _write_msr(uint32_t msr, uint64_t value);
    extern "C" uint64_t _read_msr(uint32_t msr);

    void test_lstar()
    {
    }

    void syscall::init_syscall()
    {
        _write_msr(MSR::EFER, _read_msr(MSR::EFER) | 1);
        _write_msr(MSR::SFMASK, 0x0002);
        _write_msr(MSR::LSTAR, reinterpret_cast<uint64_t>(test_lstar));

        uint64_t star_value
            = (static_cast<uint64_t>(segment_selector::KERNEL_CS >> 32)
               | static_cast<uint64_t>(segment_selector::USER_CS_NULL << 48));
        _write_msr(MSR::STAR, star_value);
        _write_msr(MSR::CSTAR, 0);
    }

    void syscall::handle_syscall()
    {
    }

}
