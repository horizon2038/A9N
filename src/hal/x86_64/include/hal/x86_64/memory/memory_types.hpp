#ifndef A9N_X86_64_MEMORY_TYPES_HPP
#define A9N_X86_64_MEMORY_TYPES_HPP

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/cpuid.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <kernel/memory/memory_type.hpp>

namespace a9n::hal::x86_64
{
    inline constexpr a9n::word PAGE_CACHE_MASK = (1ULL << 3) | (1ULL << 4);

    inline hal_result configure_cpu_memory_types()
    {
        const auto features = try_get_cpuid(cpuid_leaf::PROCESSOR_INFO_AND_FEATURE_BITS, 0);
        if (!features || !(features.unwrap().rdx & (1U << 16)))
        {
            return hal_error::UNSUPPORTED;
        }

        // Only establish the CPU's interpretation of user mapping flags.
        // Leave firmware MTRRs and APIC memory attributes unchanged.
        // No Device discovery, direct-map rewrite, or shared initialization state.
        const auto pat        = _read_msr(msr::PAT);
        const auto target_pat = (pat & ~(0xffULL | (0xffULL << 8) | (0xffULL << 24))) | 6ULL
                              | (1ULL << 8); // PAT[0]=WB, PAT[1]=WC, PAT[3]=strong UC.
        if (pat == target_pat)
        {
            return {};
        }

        const auto cr0 = _read_cr0();
        const auto cr4 = _read_cr4();
        if (cr4 & cr4_flag::PCID)
        {
            return hal_error::UNSUPPORTED;
        }
        _write_cr0((cr0 | cr0_flag::CACHE_DISABLE) & ~cr0_flag::NOT_WRITE_THROUGH);
        _writeback_invalidate_caches();
        _write_cr4(cr4 & ~cr4_flag::PAGE_GLOBAL);
        _write_msr(msr::PAT, target_pat);
        _write_cr3(_read_cr3());
        _writeback_invalidate_caches();
        _write_cr4(cr4);
        _write_cr0(cr0);
        return {};
    }

    inline constexpr a9n::word mapping_cache_flags(a9n::word attribute)
    {
        switch (kernel::get_mapping_flag(attribute))
        {
            case kernel::mapping_flag::NORMAL :
                return 0;
            case kernel::mapping_flag::BUFFERED :
                return 1ULL << 3;
            case kernel::mapping_flag::UNCACHED :
                return PAGE_CACHE_MASK;
        }
        return 0; // Reserved values are rejected before mapping.
    }
}

#endif
