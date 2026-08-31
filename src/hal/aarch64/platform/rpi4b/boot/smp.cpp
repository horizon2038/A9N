#include <hal/aarch64/platform.hpp>

#include <kernel/memory/memory.hpp>
#include <kernel/utility/logger.hpp>

namespace a9n::hal::aarch64::platform
{
    hal_result start_cpu(const cpu_description &cpu, a9n::physical_address entry_address)
    {
        if (cpu.enable_method != cpu_enable_method::SPIN_TABLE || !cpu.release_address
            || !entry_address)
        {
            return hal_error::UNSUPPORTED;
        }

        auto *release = a9n::kernel::physical_to_virtual_pointer<volatile a9n::word>(
            cpu.release_address
        );
        a9n::kernel::utility::logger::printh(
            "Spin-table CPU release: target=0x%016llx, release-address=0x%016llx, entry=0x%016llx\n",
            cpu.mpidr,
            cpu.release_address,
            entry_address
        );
        *release = entry_address;
        asm volatile("dc cvac, %0; dsb sy; sev" : : "r"(release) : "memory");
        a9n::kernel::utility::logger::printh(
            "Spin-table release event sent for target=0x%016llx.\n",
            cpu.mpidr
        );
        return {};
    }
}
