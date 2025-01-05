#include "hal/hal_result.hpp"
#include "hal/x86_64/virtualization/vmx/vmcs_field.hpp"
#include "hal/x86_64/virtualization/vmx/vmcs_region.hpp"
#include "hal/x86_64/virtualization/vmx/vmx_result.hpp"
#include "kernel/utility/logger.hpp"
#include <hal/x86_64/virtualization/vmx/vmx.hpp>

#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/virtualization/vmx/vmcs.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_msr.hpp>

namespace a9n::hal::x86_64
{
    extern "C" void _vmxon(a9n::physical_address *vmxon_region);
    extern "C" void _vmlaunch(void);

    struct vmx_control_registers
    {
        uint32_t cr0_fixed_0;
        uint32_t cr0_fixed_1;

        uint32_t cr4_fixed_0;
        uint32_t cr4_fixed_1;
    };

    static hal_result            check_support_vmx(void);
    static hal_result            enable_vmx_outside_smx(void);
    static hal_result            enable_vmx_control_registers(void);
    static vmx_control_registers get_vmx_control_registers(void);
    static hal_result            configure_vmx_control_registers(const vmx_control_registers &crs);
    static hal_result            configure_vmxon_region(vmxon_region &region);
    static vmx_result<>          vmxon(vmxon_region &region);

    inline static vmx_result<> log_vm_instruction_error(vmx_error error)
    {
        return vm_read(vmcs_fields::VM_INSTRUCTION_ERROR)
            .and_then(
                [&](uint64_t raw) -> vmx_result<>
                {
                    a9n::kernel::utility::logger::printh(
                        "VM instruction error : "
                        "%s\n",
                        vm_instruction_error_to_string(static_cast<vm_instruction_error>(raw))
                    );

                    return error;
                }
            );
    }

    hal_result run_test_vm(void)
    {
        a9n::kernel::utility::logger::printh("init VMCS Region ...\n");
        return try_get_vmcs_revision_id()
            .transform_error(
                [](hal_error e) -> vmx_error
                {
                    return vmx_error::VM_FAIL_INVALID;
                }
            )
            .and_then(
                [](uint32_t revision_id) -> vmx_result<>
                {
                    return init_vmcs(vmcs_region_core, revision_id);
                }
            )
            .and_then(
                // configure VM
                [](void) -> vmx_result<>
                {
                    _vmlaunch();
                    return check_vmx_result().or_else(log_vm_instruction_error);
                }
            )
            .and_then(
                // start VM
                [](void) -> vmx_result<>
                {
                    _vmlaunch();
                    return check_vmx_result().or_else(log_vm_instruction_error);
                }
            )
            .or_else(
                []([[maybe_unused]] vmx_error e) -> hal_result
                {
                    return hal_error::UNEXPECTED;
                }
            );
    }

    hal_result enable_vmx(void)
    {
        a9n::kernel::utility::logger::printh("enable VMX ...\n");
        return check_support_vmx()
            .and_then(enable_vmx_control_registers)
            .and_then(
                [](void) -> hal_result
                {
                    return configure_vmxon_region(vmxon_region_core);
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    return vmxon(vmxon_region_core)
                        .or_else(
                            []([[maybe_unused]] vmx_error e) -> hal_result
                            {
                                return hal_error::UNEXPECTED;
                            }
                        )
                        .and_then(
                            [](void) -> hal_result
                            {
                                a9n::kernel::utility::logger::printn(
                                    R"(
__      _________          
\ \    / /__   __|         
 \ \  / /   | |________  __
  \ \/ /    | |______\ \/ /
   \  /     | |       >  < 
    \/      |_|      /_/\_\
                       
                                        )"
                                );
                                a9n::kernel::utility::logger::split();

                                return {};
                            }
                        );
                }
            );
    }

    static hal_result check_support_vmx(void)
    {
        using a9n::kernel::utility::logger;

        logger::printh("check support VMX ...\n");

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

                        return enable_vmx_outside_smx();
                    }

                    return {};
                }
            );
    }

    static hal_result enable_vmx_outside_smx(void)
    {
        auto value = _read_msr(msr::FEATURE_CONTROL);
        if (!value)
        {
            return hal_error::UNSUPPORTED;
        }

        _write_msr(
            msr::FEATURE_CONTROL,
            value | liba9n::enum_cast(msr_feature_control::ENABLE_VMX_OUTSIDE_SMX)
        );

        return {};
    }

    static hal_result enable_vmx_control_registers(void)
    {
        a9n::kernel::utility::logger::printh("enable VMX control registers ...\n");

        auto crs = get_vmx_control_registers();

        return configure_vmx_control_registers(crs);
    }

    static vmx_control_registers get_vmx_control_registers(void)
    {
        return {
            .cr0_fixed_0 = static_cast<uint32_t>(_read_msr(msr::VMX_CR0_FIXED_0)),
            .cr0_fixed_1 = static_cast<uint32_t>(_read_msr(msr::VMX_CR0_FIXED_1)),
            .cr4_fixed_0 = static_cast<uint32_t>(_read_msr(msr::VMX_CR4_FIXED_0)),
            .cr4_fixed_1 = static_cast<uint32_t>(_read_msr(msr::VMX_CR4_FIXED_1)),
        };
    }

    static hal_result configure_vmx_control_registers(const vmx_control_registers &crs)
    {
        uint32_t target_cr0 = (_read_cr0() | crs.cr0_fixed_0) & crs.cr0_fixed_1;
        uint32_t target_cr4 = (_read_cr4() | crs.cr4_fixed_0) & crs.cr4_fixed_1;

        if (!target_cr0 || !target_cr4)
        {
            return hal_error::UNEXPECTED;
        }

        _write_cr0(target_cr0);
        _write_cr4(target_cr4);

        return {};
    }

    static hal_result configure_vmxon_region(vmxon_region &region)
    {
        a9n::kernel::utility::logger::printh("configure VMXON region ...\n", region);

        return try_get_vmcs_revision_id().and_then(
            [&](uint32_t revision_id) -> hal_result
            {
                a9n::kernel::utility::logger::printh("VMCS revision : 0x%08llx \n", revision_id);
                region.revision_id = revision_id;
                return {};
            }
        );
    }

    // `vmxon` : call `_vmxon` described by NASM source codes
    static vmx_result<> vmxon(vmxon_region &region)
    {
        a9n::kernel::utility::logger::printh("vmxon [0x%016llx] ...\n", region);

        auto region_address = reinterpret_cast<a9n::virtual_address>(&region);
        if (region_address % a9n::PAGE_SIZE != 0)
        {
            return vmx_error::VM_FAIL_VALID;
        }

        auto region_physical_address = a9n::kernel::virtual_to_physical_address(region_address);

        _vmxon(&region_physical_address);

        return check_vmx_result().or_else(
            [](vmx_error e) -> vmx_result<>
            {
                a9n::kernel::utility::logger::printh("vmxon failed : [%s]\n", vmx_error_to_string(e));
                return e;
            }
        );
    }
}
