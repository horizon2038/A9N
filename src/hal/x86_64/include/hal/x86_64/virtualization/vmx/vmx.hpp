#ifndef A9N_HAL_X86_64_VMX_HPP
#define A9N_HAL_X86_64_VMX_HPP

#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/common/enum.hpp>
#include <liba9n/libc/string.hpp>
#include <liba9n/result/result.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/cpuid.hpp>
#include <hal/x86_64/arch/msr.hpp>

namespace a9n::hal::x86_64
{
    inline hal_result enable_vmx(void)
    {
        using a9n::kernel::utility::logger;

        return try_get_vendor_id()
            .and_then(
                [](vendor_id id) -> hal_result
                {
                    if (liba9n::std::memcmp(vendor_identifier::INTEL, id.data(), sizeof(vendor_id))
                        != 0)
                    {
                        logger::printh("VMX : unsupported vendor id [%s]\n", id.data());
                        return hal_error::UNSUPPORTED;
                    }

                    return {};
                }
            )
            .and_then(try_get_processor_info_and_feature)
            .and_then(
                []([[maybe_unused]] cpuid_info info) -> hal_result
                {
                    if ((info.rcx & liba9n::enum_cast(cpuid_feature_information::VMX)) == 0)
                    {
                        logger::printh("VMX : VMX bit is invalid\n");
                        return hal_error::UNSUPPORTED;
                    }

                    return {};
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    auto value = _read_msr(msr::FEATURE_CONTROL);
                    if ((value & liba9n::enum_cast(msr_feature_control::ENABLE_VMX_OUTSIDE_SMX)) == 0)
                    {
                        if ((value & liba9n::enum_cast(msr_feature_control::LOCK_BIT)) == 0)
                        {
                            logger::printh("VMX : MSR is locked\n");
                            return hal_error::UNSUPPORTED;
                        }

                        // enable VMXON outside SMX
                        _write_msr(
                            msr::FEATURE_CONTROL,
                            value | liba9n::enum_cast(msr_feature_control::ENABLE_VMX_OUTSIDE_SMX)
                        );
                    }

                    return {};
                }
            );
    }
}

#endif
