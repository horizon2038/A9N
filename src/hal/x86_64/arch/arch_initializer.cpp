#include "hal/interface/memory_manager.hpp"
#include "kernel/types.hpp"
#include <hal/x86_64/arch/arch_initializer.hpp>

#include <hal/x86_64/arch/arch_types.hpp>
#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/arch/cpuid.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/arch/smp.hpp>
#include <hal/x86_64/interrupt/apic.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>
#include <hal/x86_64/interrupt/pic.hpp>
#include <hal/x86_64/platform/acpi.hpp>
#include <hal/x86_64/systemcall/syscall.hpp>
#include <hal/x86_64/time/acpi_pm_timer.hpp>
#include <hal/x86_64/time/local_apic_timer.hpp>

// test
#include <hal/x86_64/virtualization/vmx/vmx.hpp>

#include <kernel/boot/boot_info.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/libc/string.hpp>

// temp
#include <hal/x86_64/memory/memory_manager.hpp>

namespace a9n::hal::x86_64
{
    hal_result init_sub_cores();

    hal_result arch_initializer::init_architecture(a9n::word arch_info[])
    {
        for (auto i = 0; i < 8; i++)
        {
            a9n::kernel::utility::logger::printh("arch_info [%d] : 0x%016llx\n", i, arch_info[i]);
        }

        a9n::kernel::utility::logger::split();

        return init_main_core(arch_info[0]).and_then(init_sub_cores).and_then(unmap_lower_memory_mapping);
    }

    hal_result init_main_core(a9n::physical_address rsdp_address)
    {
        using a9n::kernel::utility::logger;

        // init *main* core (i.e., BSP)
        // enable cpu features
        return init_cpu_core()
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("cpu initialization failed\n", static_cast<a9n::word>(e));
                    return e;
                }
            )
            .and_then(
                [=](void) -> liba9n::result<madt *, hal_error>
                {
                    // init ACPI
                    if (!rsdp_address)
                    {
                        return hal_error::NO_SUCH_ADDRESS;
                    }

                    acpi_core.init(a9n::kernel::physical_to_virtual_address(rsdp_address));

                    return acpi_core.current_madt();
                }
            )
            .and_then(
                [](madt *madt_base) -> hal_result
                {
                    logger::printh("init IO APIC\n");
                    return io_apic_core.init(madt_base);
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    logger::printh("disable PIC\n");
                    disable_pic();

                    logger::printh("init Local APIC\n");
                    return local_apic_core.init();
                }
            )
            .and_then(
                [](void) -> liba9n::result<fadt *, hal_error>
                {
                    logger::printh("init ACPI\n");
                    return acpi_core.current_fadt();
                }
            )
            .and_then(
                [](fadt *fadt_base) -> hal_result
                {
                    logger::printh("init ACPI PM Timer\n");
                    return acpi_pm_timer_core.init(fadt_base);
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("ACPI / APIC initialization failed : %s\n", hal_error_to_string(e));
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("init syscall\n");
                    return local_apic_timer_core.init();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh(
                        "Local APIC Timer initialization failed : %s\n",
                        hal_error_to_string(e)
                    );
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("init syscall\n");
                    return init_syscall();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("syscall initialization failed : %s\n", hal_error_to_string(e));
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    // return interrupt_core.init();
                    a9n::kernel::utility::logger::printh("init IDT handler ...\n");
                    return init_idt_handler();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("interrupt initialization failed : %s\n", hal_error_to_string(e));
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("load vendor id ...\n");
                    return try_get_vendor_id().and_then(
                        [](vendor_id id) -> hal_result
                        {
                            a9n::kernel::utility::logger::printh("vendor id : %s\n", id.data());
                            return {};
                        }
                    );
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("load cpu name ...\n");
                    return try_get_cpu_name().and_then(
                        [](cpu_name name) -> hal_result
                        {
                            a9n::kernel::utility::logger::printh("cpu name : %s\n", name.data());
                            return {};
                        }
                    );
                }
            )
            .and_then(enable_vmx);
    }

    // for smp

    // source
    extern "C" uint8_t __boot_ap_trampoline_original_start[];
    extern "C" uint8_t __boot_ap_trampoline_original_end[];

    // destination
    extern "C" uint8_t __boot_ap_trampoline_start[];
    extern "C" uint8_t __boot_ap_trampoline_end[];

    hal_result init_sub_cores()
    {
        using a9n::kernel::utility::logger;

        a9n::physical_address destination_trampoline_end
            = reinterpret_cast<a9n::physical_address>(__boot_ap_trampoline_end);
        a9n::physical_address destination_trampoline_start
            = reinterpret_cast<a9n::physical_address>(__boot_ap_trampoline_start);
        a9n::physical_address source_trampoline_start
            = reinterpret_cast<a9n::physical_address>(__boot_ap_trampoline_original_start);
        auto boot_ap_trampoline_size = destination_trampoline_end - destination_trampoline_start;

        logger::printh(
            "copy trampoline codes ... [0x%016llx - 0x%016llx) -> 0x%016llx\n",
            source_trampoline_start,
            (source_trampoline_start + boot_ap_trampoline_size),
            destination_trampoline_start
        );
        // copy trampolines!
        liba9n::std::memcpy(
            a9n::kernel::physical_to_virtual_pointer<void *>(destination_trampoline_start),
            a9n::kernel::physical_to_virtual_pointer<void *>(source_trampoline_start),
            boot_ap_trampoline_size
        );

        // get smp
        return {};

        // temp
        auto cpu_max = 16;

        for (auto i = 1; i <= cpu_max; i++)
        {
            logger::printh("starting core [%3d] ...\n", i);
            auto result = ipi_init(i).and_then(
                [=](void) -> hal_result
                {
                    // wait 20ms
                    return acpi_pm_timer_core.wait(20000)
                        .and_then(
                            [=](void) -> hal_result
                            {
                                return ipi_startup(destination_trampoline_start, i);
                            }
                        )
                        .and_then(
                            [=](void) -> hal_result
                            {
                                return acpi_pm_timer_core.wait(4000);
                            }
                        );
                }
            );
            if (!result)
            {
                return result.unwrap_error();
            }
        }

        return {};
    }

    hal_result init_sub_core()
    {
        using a9n::kernel::utility::logger;

        // long mode things
        return init_cpu_core()
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("cpu initialization failed\n", static_cast<a9n::word>(e));
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    logger::printh("disable PIC\n");
                    disable_pic();

                    logger::printh("init Local APIC\n");
                    return local_apic_core.init();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh(
                        "ACPI / APIC initialization failed : 0x%llx\n",
                        static_cast<a9n::word>(e)
                    );
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("init syscall\n");
                    return init_syscall();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("syscall initialization failed : 0x%llx\n", static_cast<a9n::word>(e));
                    return e;
                }
            );
    };
}
