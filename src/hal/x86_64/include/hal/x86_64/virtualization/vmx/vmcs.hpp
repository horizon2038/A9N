#ifndef A9N_HAL_X86_64_VMCS_HPP
#define A9N_HAL_X86_64_VMCS_HPP

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/msr.hpp>
#include <hal/x86_64/virtualization/vmx/vmcs_field.hpp>
#include <hal/x86_64/virtualization/vmx/vmcs_region.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_msr.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_result.hpp>
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

    inline vmx_result<> init_vmcs(vmcs_region &region, uint32_t revision_id)
    {
        region.configure_revision_id(revision_id);

        return vm_clear(region).and_then(
            [&](void) -> vmx_result<>
            {
                return vm_pointer_load(region);
            }
        );
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

    // Guest State
    // holds a state of guest processor;
    // load : VM Entry
    // save : VM Exit

    // Host State
    // holds a state of host processor;
    // load : VM Exit

    // VM Execution Control

    // VM Exit Control

    // VM Entry Control

    // VM Exit Information

}

#endif
