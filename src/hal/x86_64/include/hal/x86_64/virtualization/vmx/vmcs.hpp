#ifndef A9N_HAL_X86_64_VMCS_HPP
#define A9N_HAL_X86_64_VMCS_HPP

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/virtualization/vmx/vmcs_field.hpp>
#include <hal/x86_64/virtualization/vmx/vmcs_region.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_msr.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_result.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    // VM INSTCUTION ERROR NUMBERS
    // cf., Intel SDM vol. 3C 31-31, table 31-1, "VM-Instruction Error Numbers"
    //      (combined : page 3966)
    enum class vm_instruction_error : uint32_t
    {
        VMCALL_EXECUTED_IN_VMX_ROOT_OPERATION                                = 1,
        VMCLEAR_WITH_INVALID_PHYSICAL_ADDRESS                                = 2,
        VMCLEAR_WITH_VMXON_POINETR                                           = 3,
        VMLAUNCH_WITH_NON_CLEAR_VMCS                                         = 4,
        VMRESUME_WITH_NON_LAUNCHED_VMCS                                      = 5,
        VMRESUME_AFTER_VMXOFF                                                = 6,
        VM_ENTRY_WITH_INVALID_CONTROL_FIELDS                                 = 7,
        VM_ENTRY_WITH_INVALID_HOST_STATE_FIELDS                              = 8,
        VMPTRLD_WITH_INVALID_PHYSICAL_ADDRESS                                = 9,
        VMPTRLD_WITH_VMXON_POINTER                                           = 10,
        VMPTRLD_WITH_INCORRECT_VMCS_REVISION_IDENTIFIER                      = 11,
        VMREAD_VMWRITE_FROM_TO_UNSUPPORTED_VMCS_COMPONENT                    = 12,
        VMWRITE_TO_READ_ONLY_VMCS_COMPONENT                                  = 13,
        VMXON_EXECUTED_IN_VMX_ROOT_OPERATION                                 = 15,
        VM_ENTRY_WITH_INVALID_EXECUTIVE_VMCS_POINTER                         = 16,
        VM_ENTRY_WITH_NON_LAUNCHED_EXECUTIVE_VMCS                            = 17,
        VM_WNTRY_WITH_EXECUTIVE_VMCS_POINTER_NOT_VMXON_POINTER               = 18,
        VMCALL_WITH_NON_CLEAR_VMCS                                           = 19,
        VMCALL_WITH_INVALID_VM_EXIT_CONTROL_FIELDS                           = 20,
        VMCALL_WITH_INCORRECT_MSEG_REVISION_IDENTIFIER                       = 22,
        VMXOFF_UNDER_FUAL_MONITOR_TREATMENT_OF_SMIS_AND_SMM                  = 23,
        VMCALL_WITH_INVALID_SMM_MONITOR_FEATURES                             = 24,
        VME_ENTRY_WITH_INVALID_VM_EXECUTION_CONTROL_FIELDS_IN_EXECUTIVE_VMCS = 25,
        VM_ENTRY_WITH_EVENTS_BLOCKED_BY_MOV_SS                               = 26,
        INVALID_OPERAND_TO_INVEPT_INVVPID                                    = 28,
    };

    inline constexpr const char *vm_instruction_error_to_string(vm_instruction_error error)
    {
        switch (error)
        {
            using enum vm_instruction_error;

            case VMCALL_EXECUTED_IN_VMX_ROOT_OPERATION :
                return "VMCALL EXECUTED IN VMX ROOT OPERATION";

            case VMCLEAR_WITH_INVALID_PHYSICAL_ADDRESS :
                return "VMCLEAR WITH INVALID PHYSICAL ADDRESS";

            case VMCLEAR_WITH_VMXON_POINETR :
                return "VMCLEAR WITH VMXON POINETR";

            case VMLAUNCH_WITH_NON_CLEAR_VMCS :
                return "VMLAUNCH WITH NON CLEAR VMCS";

            case VMRESUME_WITH_NON_LAUNCHED_VMCS :
                return "VMRESUME WITH NON LAUNCHED VMCS";

            case VMRESUME_AFTER_VMXOFF :
                return "VMRESUME AFTER VMXOFF";

            case VM_ENTRY_WITH_INVALID_CONTROL_FIELDS :
                return "VM ENTRY WITH INVALID CONTROL FIELDS";

            case VM_ENTRY_WITH_INVALID_HOST_STATE_FIELDS :
                return "VM ENTRY WITH INVALID HOST STATE FIELDS";

            case VMPTRLD_WITH_INVALID_PHYSICAL_ADDRESS :
                return "VMPTRLD WITH INVALID PHYSICAL ADDRESS";

            case VMPTRLD_WITH_VMXON_POINTER :
                return "VMPTRLD WITH VMXON POINTER";

            case VMPTRLD_WITH_INCORRECT_VMCS_REVISION_IDENTIFIER :
                return "VMPTRLD WITH INCORRECT VMCS REVISION IDENTIFIER";

            case VMREAD_VMWRITE_FROM_TO_UNSUPPORTED_VMCS_COMPONENT :
                return "VMREAD VMWRITE FROM TO UNSUPPORTED VMCS COMPONENT";

            case VMWRITE_TO_READ_ONLY_VMCS_COMPONENT :
                return "VMWRITE TO READ ONLY VMCS COMPONENT";

            case VMXON_EXECUTED_IN_VMX_ROOT_OPERATION :
                return "VMXON EXECUTED IN VMX ROOT OPERATION";

            case VM_ENTRY_WITH_INVALID_EXECUTIVE_VMCS_POINTER :
                return "VM ENTRY WITH INVALID EXECUTIVE VMCS POINTER";

            case VM_ENTRY_WITH_NON_LAUNCHED_EXECUTIVE_VMCS :
                return "VM ENTRY WITH NON LAUNCHED EXECUTIVE VMCS";

            case VM_WNTRY_WITH_EXECUTIVE_VMCS_POINTER_NOT_VMXON_POINTER :
                return "VM WNTRY WITH EXECUTIVE VMCS POINTER NOT VMXON POINTER";

            case VMCALL_WITH_NON_CLEAR_VMCS :
                return "VMCALL WITH NON CLEAR VMCS";

            case VMCALL_WITH_INVALID_VM_EXIT_CONTROL_FIELDS :
                return "VMCALL WITH INVALID VM EXIT CONTROL FIELDS";

            case VMCALL_WITH_INCORRECT_MSEG_REVISION_IDENTIFIER :
                return "VMCALL WITH INCORRECT MSEG REVISION IDENTIFIER";

            case VMXOFF_UNDER_FUAL_MONITOR_TREATMENT_OF_SMIS_AND_SMM :
                return "VMXOFF UNDER FUAL MONITOR TREATMENT OF SMIS AND SMM";

            case VMCALL_WITH_INVALID_SMM_MONITOR_FEATURES :
                return "VMCALL WITH INVALID SMM MONITOR FEATURES";

            case VME_ENTRY_WITH_INVALID_VM_EXECUTION_CONTROL_FIELDS_IN_EXECUTIVE_VMCS :
                return "VME ENTRY WITH INVALID VM EXECUTION CONTROL FIELDS IN EXECUTIVE VMCS";

            case VM_ENTRY_WITH_EVENTS_BLOCKED_BY_MOV_SS :
                return "VM ENTRY WITH EVENTS BLOCKED BY MOV SS";

            case INVALID_OPERAND_TO_INVEPT_INVVPID :
                return "INVALID OPERAND TO INVEPT INVVPID";

            default :
                return "ERROR NOT FOUND";
        }
    }

    // unsafe!
    extern "C" void     _vmclear(a9n::physical_address *region);
    extern "C" void     _vmptrld(a9n::physical_address *region);
    extern "C" uint64_t _vmread(vmcs_field field);
    extern "C" void     _vmwrite(vmcs_field field, uint64_t value);

    inline vmx_result<> vm_clear(const vmcs_region &region);
    inline vmx_result<> vm_pointer_load(const vmcs_region &region);

    // speed-critical; inline
    inline vmx_result<uint64_t> vm_read(vmcs_field field)
    {
        auto value = _vmread(field);

        return check_vmx_result().and_then(
            [&](void) -> vmx_result<uint64_t>
            {
                return value;
            }
        );
    }

    inline vmx_result<> vm_write(vmcs_field field, uint64_t value)
    {
        _vmwrite(field, value);

        return check_vmx_result();
    }

    static liba9n::result<uint32_t, hal_error> try_get_vmcs_revision_id(void)
    {
        auto revision_id = static_cast<uint32_t>(_read_msr(msr::VMX_BASIC));
        if (!revision_id)
        {
            return hal_error::UNEXPECTED;
        }

        return revision_id;
    }

    inline vmx_result<> vm_clear(const vmcs_region &region)
    {
        auto address = a9n::kernel::virtual_to_physical_address(
            reinterpret_cast<a9n::virtual_address>(&region)
        );
        _vmclear(&address);

        return check_vmx_result();
    }

    inline vmx_result<> vm_pointer_load(const vmcs_region &region)
    {
        auto address = a9n::kernel::virtual_to_physical_address(
            reinterpret_cast<a9n::virtual_address>(&region)
        );
        _vmptrld(&address);

        return check_vmx_result();
    }

    vmx_result<> init_vmcs(vmcs_region &region, uint32_t revision_id);

    // Guest State
    // holds a state of guest processor;
    // load : VM Entry
    // save : VM Exit

    // Host State
    // holds a state of host processor;
    // load : VM Exit

    // VM Execution Control
    struct pin_based_vm_execution_contol
    {
        uint32_t all { 0 };

        constexpr pin_based_vm_execution_contol(
            bool external_interrupt,
            bool nmi,
            bool virtual_nmi,
            bool activate_vmx_preemption_timer,
            bool process_posted_interrupts
        )
        {
            all |= (external_interrupt ? 1 : 0) << 0;
            all |= (nmi ? 1 : 0) << 3;
            all |= (virtual_nmi ? 1 : 0) << 5;
            all |= (activate_vmx_preemption_timer ? 1 : 0) << 6;
            all |= (process_posted_interrupts ? 1 : 0) << 7;
        }

        constexpr bool external_interrupt(void) const
        {
            return (all >> 0) & 1;
        }

        constexpr bool nmi(void) const
        {
            return (all >> 3) & 1;
        }

        constexpr bool virtual_nmi(void) const
        {
            return (all >> 5) & 1;
        }

        constexpr bool activate_vmx_preemption_timer(void) const
        {
            return (all >> 6) & 1;
        }

        constexpr bool process_posted_interrupts(void) const
        {
            return (all >> 7) & 1;
        }

        constexpr void configure_external_interrupt(bool value)
        {
            all |= (value ? 1 : 0) << 0;
        }

        constexpr void configure_nmi(bool value)
        {
            all |= (value ? 1 : 0) << 3;
        }

        constexpr void configure_virtual_nmi(bool value)
        {
            all |= (value ? 1 : 0) << 5;
        }

        constexpr void configure_activate_vmx_preemption_timer(bool value)
        {
            all |= (value ? 1 : 0) << 6;
        }

        constexpr void configure_process_posted_interrupts(bool value)
        {
            all |= (value ? 1 : 0) << 7;
        }
    };

    struct processor_based_vm_execution_control
    {
        uint32_t all { 0 };

        constexpr processor_based_vm_execution_control(
            bool interrupt_window_exiting,
            bool use_tsc_offsetting,
            bool hlt_exiting,
            bool invd_exiting,
            bool mwait_exiting,
            bool rdpmc_exiting,
            bool rdtsc_exiting,
            bool cr3_load_exiting,
            bool cr3_store_exiting,
            bool cr8_load_exiting,
            bool cr8_store_exiting,
            bool use_tpr_shadow,
            bool nmi_window_exiting,
            bool mov_dr_exiting,
            bool unconditional_io_exiting,
            bool use_io_bitmaps,
            bool monitor_trap_flag,
            bool use_msr_bitmaps,
            bool monitor_exiting,
            bool pause_exiting,
            bool activate_secondary_controls
        )
        {
            all |= (interrupt_window_exiting ? 1 : 0) << 2;
            all |= (use_tsc_offsetting ? 1 : 0) << 3;
            all |= (hlt_exiting ? 1 : 0) << 7;
            all |= (invd_exiting ? 1 : 0) << 9;
            all |= (mwait_exiting ? 1 : 0) << 10;
            all |= (rdpmc_exiting ? 1 : 0) << 11;
            all |= (rdtsc_exiting ? 1 : 0) << 12;
            all |= (cr3_load_exiting ? 1 : 0) << 15;
            all |= (cr3_store_exiting ? 1 : 0) << 16;
            all |= (cr8_load_exiting ? 1 : 0) << 19;
            all |= (cr8_store_exiting ? 1 : 0) << 20;
            all |= (use_tpr_shadow ? 1 : 0) << 21;
            all |= (nmi_window_exiting ? 1 : 0) << 22;
            all |= (mov_dr_exiting ? 1 : 0) << 23;
            all |= (unconditional_io_exiting ? 1 : 0) << 24;
            all |= (use_io_bitmaps ? 1 : 0) << 25;
            all |= (monitor_trap_flag ? 1 : 0) << 27;
            all |= (use_msr_bitmaps ? 1 : 0) << 28;
            all |= (monitor_exiting ? 1 : 0) << 29;
            all |= (pause_exiting ? 1 : 0) << 30;
            all |= (activate_secondary_controls ? 1 : 0) << 31;
        }

        constexpr bool interrupt_window_exiting(void) const
        {
            return (all >> 2) & 1;
        }

        constexpr bool use_tsc_offsetting(void) const
        {
            return (all >> 3) & 1;
        }

        constexpr bool hlt_exiting(void) const
        {
            return (all >> 7) & 1;
        }

        constexpr bool invd_exiting(void) const
        {
            return (all >> 9) & 1;
        }

        constexpr bool mwait_exiting(void) const
        {
            return (all >> 10) & 1;
        }

        constexpr bool rdpmc_exiting(void) const
        {
            return (all >> 11) & 1;
        }

        constexpr bool rdtsc_exiting(void) const
        {
            return (all >> 12) & 1;
        }

        constexpr bool cr3_load_exiting(void) const
        {
            return (all >> 15) & 1;
        }

        constexpr bool cr3_store_exiting(void) const
        {
            return (all >> 16) & 1;
        }

        constexpr bool cr8_load_exiting(void) const
        {
            return (all >> 19) & 1;
        }

        constexpr bool cr8_store_exiting(void) const
        {
            return (all >> 20) & 1;
        }

        constexpr bool use_tpr_shadow(void) const
        {
            return (all >> 21) & 1;
        }

        constexpr bool nmi_window_exiting(void) const
        {
            return (all >> 22) & 1;
        }

        constexpr bool mov_dr_exiting(void) const
        {
            return (all >> 23) & 1;
        }

        constexpr bool unconditional_io_exiting(void) const
        {
            return (all >> 24) & 1;
        }

        constexpr bool use_io_bitmaps(void) const
        {
            return (all >> 25) & 1;
        }

        constexpr bool monitor_trap_flag(void) const
        {
            return (all >> 27) & 1;
        }

        constexpr bool use_msr_bitmaps(void) const
        {
            return (all >> 28) & 1;
        }

        constexpr bool monitor_exiting(void) const
        {
            return (all >> 29) & 1;
        }

        constexpr bool pause_exiting(void) const
        {
            return (all >> 30) & 1;
        }

        constexpr bool activate_secondary_controls(void) const
        {
            return (all >> 31) & 1;
        }

        constexpr void configure_interrupt_window_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 2;
        }

        constexpr void configure_use_tsc_offsetting(bool value)
        {
            all |= (value ? 1 : 0) << 3;
        }

        constexpr void configure_hlt_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 7;
        }

        constexpr void configure_invd_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 9;
        }

        constexpr void configure_mwait_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 10;
        }

        constexpr void configure_rdpmc_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 11;
        }

        constexpr void configure_rdtsc_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 12;
        }

        constexpr void configure_cr3_load_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 15;
        }

        constexpr void configure_cr3_store_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 16;
        }

        constexpr void configure_cr8_load_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 19;
        }

        constexpr void configure_cr8_store_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 20;
        }

        constexpr void configure_use_tpr_shadow(bool value)
        {
            all |= (value ? 1 : 0) << 21;
        }

        constexpr void configure_nmi_window_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 22;
        }

        constexpr void configure_mov_dr_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 23;
        }

        constexpr void configure_unconditional_io_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 24;
        }

        constexpr void configure_use_io_bitmaps(bool value)
        {
            all |= (value ? 1 : 0) << 25;
        }

        constexpr void configure_monitor_trap_flag(bool value)
        {
            all |= (value ? 1 : 0) << 27;
        }

        constexpr void configure_use_msr_bitmaps(bool value)
        {
            all |= (value ? 1 : 0) << 28;
        }

        constexpr void configure_monitor_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 29;
        }

        constexpr void configure_pause_exiting(bool value)
        {
            all |= (value ? 1 : 0) << 30;
        }

        constexpr void configure_activate_secondary_controls(bool value)
        {
            all |= (value ? 1 : 0) << 31;
        }
    };

    // VM Exit Control

    // VM Entry Control
    struct vm_entry_control
    {
        uint32_t all { 0 };
    };

    // VM Exit Information

}

#endif
