#include <hal/x86_64/virtualization/vmx/vmcs.hpp>

#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/virtualization/vmx/vmcs_field.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    auto concrete_vm_write(vmcs_field field, uint64_t value) -> decltype(auto)
    {
        return [=](void) -> vmx_result<>
        {
            return vm_write(field, value)
                .transform_error(
                    [](vmx_error e) -> vmx_error
                    {
                        DEBUG_LOG("VMCS write error : %s", vmx_error_to_string(e));
                        return e;
                    }
                );
        };
    };

    inline vmx_result<> init_vmcs(vmcs_region &region, uint32_t revision_id)
    {
        region.configure_revision_id(revision_id);

        auto load_region = [&](void) -> vmx_result<>
        {
            return vm_pointer_load(region);
        };

        auto configure_vm_execution_control = [](void) -> vmx_result<>
        {
            // 1. configure pin-based vm-execution control
            // - external-interrupt + nmi
            // 2. configure processor-based vm-execution control
            return {};
        };

        auto configure_host_state = [](void) -> vmx_result<>
        {
            auto configure_common_registers = [](void) -> vmx_result<>
            {
                return vm_write(vmcs_fields::HOST_IA32_PAT, _read_msr(msr::PAT))
                    .and_then(concrete_vm_write(vmcs_fields::HOST_IA32_EFER, _read_msr(msr::EFER)));
            };

            auto configure_control_registers = [](void) -> vmx_result<>
            {
                return vm_write(vmcs_fields::HOST_CR0, _read_cr0())
                    .and_then(concrete_vm_write(vmcs_fields::HOST_CR3, _read_cr3()))
                    .and_then(concrete_vm_write(vmcs_fields::HOST_CR4, _read_cr4()));
            };

            auto configure_segment_registers = [](void) -> vmx_result<>
            {
                // clang-format off
                return vm_write(vmcs_fields::HOST_CS_SELECTOR, segment_selector::KERNEL_CS)
                    .and_then(concrete_vm_write(vmcs_fields::HOST_DS_SELECTOR, segment_selector::KERNEL_DS))
                    .and_then(concrete_vm_write(vmcs_fields::HOST_ES_SELECTOR, segment_selector::KERNEL_DS))
                    .and_then(concrete_vm_write(vmcs_fields::HOST_SS_SELECTOR, segment_selector::KERNEL_DS))
                    .and_then(concrete_vm_write(vmcs_fields::HOST_FS_SELECTOR, 0))
                    .and_then(concrete_vm_write(vmcs_fields::HOST_GS_SELECTOR, 0))
                    .and_then(concrete_vm_write(vmcs_fields::HOST_TR_SELECTOR, segment_selector::KERNEL_TSS))
                    // clang-format on
                    // configure bases
                    .and_then(concrete_vm_write(
                        vmcs_fields::HOST_GDTR_BASE,
                        reinterpret_cast<uint64_t>(
                            &(a9n::hal::x86_64::current_arch_local_variable().unwrap()->gdt)
                        )
                    ))
                    .and_then(concrete_vm_write(
                        vmcs_fields::HOST_IDTR_BASE,
                        reinterpret_cast<uint64_t>(
                            &(a9n::hal::x86_64::current_arch_local_variable().unwrap()->idt)
                        )
                    ))
                    .and_then(concrete_vm_write(
                        vmcs_fields::HOST_TR_BASE,
                        reinterpret_cast<uint64_t>(
                            &(a9n::hal::x86_64::current_arch_local_variable().unwrap()->tss)
                        )
                    ));
            };

            auto configure_sysenter_registers = [&](void) -> vmx_result<>
            {
                return vm_write(vmcs_fields::HOST_IA32_SYSENTER_CS, segment_selector::KERNEL_CS)
                    // clang-format off
                    .and_then(concrete_vm_write(vmcs_fields::HOST_IA32_SYSENTER_EIP,0 /* syscall handler */))
                    // TODO: TSS->RSP0
                    .and_then(concrete_vm_write(vmcs_fields::HOST_IA32_SYSENTER_ESP, 0 /* kernel stack */));
                // clang-format on
            };

            auto configure_host_state_registers = [&](void) -> vmx_result<>
            {
                return vm_write(vmcs_fields::HOST_RIP, 0 /* vm-exit handler */)
                    .and_then(concrete_vm_write(vmcs_fields::HOST_RSP, 0 /* vm-exit stack */));
            };

            return configure_common_registers()
                .and_then(configure_control_registers)
                .and_then(configure_segment_registers)
                .and_then(configure_host_state_registers)
                .and_then(configure_sysenter_registers);
        };

        auto configure_guest_state = [&](void) -> vmx_result<>
        {
            uint64_t initial_value = ~static_cast<uint64_t>(0);
            return vm_write(vmcs_fields::VMCS_LINK_POINTER, initial_value)
                .and_then(concrete_vm_write(vmcs_fields::VMCS_LINK_POINTER_HIGH, initial_value));
        };

        auto configure_vm_entry_control = [&](void) -> vmx_result<>
        {
            return {};
        };

        auto configure_vm_exit_control = [&](void) -> vmx_result<>
        {
            return {};
        };

        return vm_clear(region)
            .and_then(load_region)
            .and_then(configure_vm_execution_control)
            .and_then(configure_host_state)
            .and_then(configure_guest_state)
            .and_then(configure_vm_entry_control)
            .and_then(configure_vm_exit_control)
            // dummy
            .and_then(concrete_vm_write(vmcs_fields::GUEST_IA32_DEBUGCTL, segment_selector::KERNEL_DS));
    }

    inline vmx_result<> update_vmcs_host_state_per_cpu(vmcs_region &region)
    {
        // set what should be set for cpu-local and VMCS host state.
        // basically, the value stored in cpu-local variable is the target.
        return current_arch_local_variable()
            .transform_error(
                [](hal_error e) -> vmx_error
                {
                    return vmx_error::VM_FAIL_INVALID;
                }
            )
            .and_then(
                [&](arch_cpu_local_variable *variable) -> vmx_result<>
                {
                    // clang-format off
                    return vm_write(vmcs_fields::HOST_CR3, _read_cr3())
                        .and_then(concrete_vm_write(vmcs_fields::HOST_GS_BASE, _read_gs_base()))
                        .and_then(concrete_vm_write(vmcs_fields::HOST_FS_BASE, _read_fs_base()))
                        .and_then(concrete_vm_write(vmcs_fields::HOST_GDTR_BASE, reinterpret_cast<uint64_t>(&variable->gdt)))
                        .and_then(concrete_vm_write(vmcs_fields::HOST_IDTR_BASE, reinterpret_cast<uint64_t>(&variable->idt)))
                        .and_then(concrete_vm_write(vmcs_fields::HOST_TR_BASE, reinterpret_cast<uint64_t>(&variable->tss)));
                    // clang-format on
                }
            );
    }
}
