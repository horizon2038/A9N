#ifndef A9N_HAL_X86_64_VMX_HPP
#define A9N_HAL_X86_64_VMX_HPP

#include <kernel/types.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/common/enum.hpp>
#include <liba9n/libc/string.hpp>
#include <liba9n/result/result.hpp>

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/cpuid.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_result.hpp>

namespace a9n::hal::x86_64
{
    struct vmxon_region
    {
        uint32_t revision_id;
        uint8_t  data[(a9n::PAGE_SIZE - sizeof(uint32_t))];
    } __attribute__((packed));

    // vmxon_region must have page size (4KiB)
    static_assert(sizeof(vmxon_region) == a9n::PAGE_SIZE);
    alignas(a9n::PAGE_SIZE) inline vmxon_region vmxon_region_core;

    inline hal_result   check_support_vmx(void);
    inline vmx_result<> vmxon(vmxon_region &region);
    extern "C" void     _vmxon(a9n::physical_address vmxon_region);

    inline hal_result enable_vmx(void)
    {
        return check_support_vmx().and_then(
            [](void) -> hal_result
            {
                return vmxon(vmxon_region_core)
                    .or_else(
                        []([[maybe_unused]] vmx_error e) -> hal_result
                        {
                            return hal_error::UNEXPECTED;
                        }
                    );
            }
        );
    }

    inline hal_result check_support_vmx(void)
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

    inline hal_result enable_vmx_control_registers(void)
    {
        uint64_t current_cr4  = _read_cr4();
        current_cr4          |= cr4_flag::VIRTUAL_MACHINE_EXTENSION;
        _write_cr4(current_cr4);

        return {};
    }

    // `vmxon` : call `_vmxon` described by NASM source codes
    inline vmx_result<> vmxon(vmxon_region &region)
    {
        auto region_address = reinterpret_cast<a9n::virtual_address>(&region);
        if (region_address % a9n::PAGE_SIZE != 0)
        {
            return vmx_error::VM_FAIL_VALID;
        }

        _vmxon(region_address);

        return check_vmx_result();
    }

}

#endif
