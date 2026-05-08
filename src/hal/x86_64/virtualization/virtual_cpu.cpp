#include <hal/interface/virtualize.hpp>

#include <hal/interface/cpu.hpp>
#include <hal/x86_64/virtualization/virtual_cpu.hpp>
#include <hal/x86_64/virtualization/vmx/vmcs.hpp>
#include <hal/x86_64/virtualization/vmx/vmx.hpp>
#include <hal/x86_64/virtualization/vmx/vmx_result.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal
{
    hal_result try_init_virtual_cpu(a9n::kernel::virtual_cpu &cpu)
    {
        return x86_64::try_get_vmcs_revision_id().and_then(
            [&](uint32_t revision_id) -> hal_result
            {
                x86_64::arch_virtual_cpu &vcpu
                    = *(reinterpret_cast<x86_64::arch_virtual_cpu *>(cpu.context.data()));
                x86_64::vmcs_region &region = vcpu.vmcs;
                return x86_64::init_vmcs(region, revision_id)
                    .transform_error(
                        [](x86_64::vmx_error e) -> hal_error
                        {
                            DEBUG_LOG("VMX error : %s\n", x86_64::vmx_error_to_string(e));
                            return hal_error::UNEXPECTED;
                        }
                    );
            }
        );
    }

    hal_result enter_virtual_machine(a9n::kernel::virtual_cpu &cpu)
    {
        return {};
    }

    hal_result inject_virtual_irq(a9n::kernel::virtual_cpu &cpu, a9n::word irq)
    {
        return {};
    }
}
