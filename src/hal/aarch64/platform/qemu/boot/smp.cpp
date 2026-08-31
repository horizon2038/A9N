#include <hal/aarch64/platform.hpp>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::aarch64::platform
{
    namespace
    {
        constexpr a9n::word PSCI_CPU_ON_64 = 0xc4000003;

        a9n::sword psci_cpu_on_hvc(
            a9n::word target_cpu,
            a9n::word entry_address,
            a9n::word context
        )
        {
            register a9n::word x0 asm("x0") = PSCI_CPU_ON_64;
            register a9n::word x1 asm("x1") = target_cpu;
            register a9n::word x2 asm("x2") = entry_address;
            register a9n::word x3 asm("x3") = context;
            asm volatile(
                "hvc #0"
                : "+r"(x0)
                : "r"(x1), "r"(x2), "r"(x3)
                : "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13",
                  "x14", "x15", "x16", "x17", "cc", "memory"
            );
            return static_cast<a9n::sword>(x0);
        }

        a9n::sword psci_cpu_on_smc(
            a9n::word target_cpu,
            a9n::word entry_address,
            a9n::word context
        )
        {
            register a9n::word x0 asm("x0") = PSCI_CPU_ON_64;
            register a9n::word x1 asm("x1") = target_cpu;
            register a9n::word x2 asm("x2") = entry_address;
            register a9n::word x3 asm("x3") = context;
            asm volatile(
                "smc #0"
                : "+r"(x0)
                : "r"(x1), "r"(x2), "r"(x3)
                : "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13",
                  "x14", "x15", "x16", "x17", "cc", "memory"
            );
            return static_cast<a9n::sword>(x0);
        }
    }

    hal_result start_cpu(const cpu_description &cpu, a9n::physical_address entry_address)
    {
        if (cpu.enable_method != cpu_enable_method::PSCI || !entry_address)
        {
            return hal_error::UNSUPPORTED;
        }

        a9n::sword result = -1;
        const char *conduit = "none";
        switch (discovered_psci_conduit)
        {
            case psci_conduit::HVC :
                conduit = "hvc";
                a9n::kernel::utility::logger::printh(
                    "PSCI CPU_ON: conduit=%s, target=0x%016llx, entry=0x%016llx\n",
                    conduit,
                    cpu.mpidr,
                    entry_address
                );
                result = psci_cpu_on_hvc(cpu.mpidr, entry_address, cpu.mpidr);
                break;
            case psci_conduit::SMC :
                conduit = "smc";
                a9n::kernel::utility::logger::printh(
                    "PSCI CPU_ON: conduit=%s, target=0x%016llx, entry=0x%016llx\n",
                    conduit,
                    cpu.mpidr,
                    entry_address
                );
                result = psci_cpu_on_smc(cpu.mpidr, entry_address, cpu.mpidr);
                break;
            default :
                a9n::kernel::utility::logger::printh(
                    "PSCI CPU_ON cannot start target=0x%016llx: no conduit.\n",
                    cpu.mpidr
                );
                return hal_error::UNSUPPORTED;
        }
        a9n::kernel::utility::logger::printh(
            "PSCI CPU_ON result: target=0x%016llx, conduit=%s, status=%lld\n",
            cpu.mpidr,
            conduit,
            result
        );
        return result == 0 ? hal_result {} : hal_result { hal_error::NO_SUCH_DEVICE };
    }
}
