#ifndef A9N_HAL_X86_64_VCPU_HPP
#define A9N_HAL_X86_64_VCPU_HPP

#include <hal/x86_64/virtualization/vmx/vmcs_region.hpp>
#include <liba9n/libcxx/array>

namespace a9n::hal::x86_64
{
    struct arch_virtual_cpu
    {
        vmcs_region vmcs;

        struct msr_area_entry
        {
            uint32_t index;
            uint32_t reserved;
            uint64_t value;
        };

        alignas(a9n::PAGE_SIZE) liba9n::std::array<msr_area_entry, 256> host_msr_area;
        alignas(a9n::PAGE_SIZE) liba9n::std::array<msr_area_entry, 256> guest_msr_area;

        struct segment_register
        {
            a9n::word fS;
            a9n::word gs;
            a9n::word shadow_gs;
        } host_segment_register, guest_segment_register;

        struct system_call_register
        {
            a9n::word STAR;
            a9n::word LSTAR;
            a9n::word CSTAR;
            a9n::word SFMASK;
        } guest_system_call_register;
    };
}

#endif
