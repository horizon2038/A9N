#ifndef A9N_HAL_X86_64_VMX_RESULT_HPP
#define A9N_HAL_X86_64_VMX_RESULT_HPP

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/rflags.hpp>

namespace a9n::hal::x86_64
{
    enum class vmx_error
    {
        VM_FAIL_VALID,
        VM_FAIL_INVALID,
    };

    template<typename T = void>
    using vmx_result = liba9n::result<T, vmx_error>;

    inline vmx_result<> check_vmx_result(void)
    {
        auto rflags = _read_rflags();
        if ((rflags & rflags_flag::CARRY) != 0)
        {
            return vmx_error::VM_FAIL_INVALID;
        }
        if ((rflags & rflags_flag::ZERO_FLAG) != 0)
        {
            return vmx_error::VM_FAIL_VALID;
        }

        return {};
    }

    inline constexpr const char *vmx_error_to_string(vmx_error error)
    {
        switch (error)
        {
            using enum vmx_error;

            case vmx_error::VM_FAIL_VALID :
                return "VM FAIL VALID";

            case VM_FAIL_INVALID :
                return "VM FAIL INVALID";

            default :
                return "ERROR NOT FOUND";
        }
    }

}

#endif
