#include <hal/x86_64/arch/arch_initializer.hpp>

#include <kernel/config.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/types.hpp>

#include <hal/hal_result.hpp>
#include <hal/interface/cpu.hpp>
#include <hal/interface/lock.hpp>
#include <hal/x86_64/arch/arch_types.hpp>
#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/arch/cpuid.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/arch/smp.hpp>
#include <hal/x86_64/interrupt/apic.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>
#include <hal/x86_64/interrupt/pic.hpp>
#include <hal/x86_64/io/serial.hpp>
#include <hal/x86_64/memory/paging.hpp>
#include <hal/x86_64/platform/acpi.hpp>
#include <hal/x86_64/systemcall/syscall.hpp>
#include <hal/x86_64/time/acpi_pm_timer.hpp>
#include <hal/x86_64/time/local_apic_timer.hpp>

// test
#include <hal/x86_64/virtualization/vmx/vmx.hpp>

#include <kernel/boot/boot_info.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/process/lock.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/libc/string.hpp>

// temp
#include <hal/x86_64/arch/smp.hpp>
#include <hal/x86_64/process/idle.hpp>
#include <kernel/process/cpu.hpp>

namespace a9n::hal::x86_64
{
    namespace
    {
        hal_result unmap_lower_memory_mapping(void)
        {
            a9n::physical_address *kernel_top_page_table
                = reinterpret_cast<a9n::physical_address *>(&__kernel_pml4);
            if (!kernel_top_page_table)
            {
                return hal_error::NO_SUCH_ADDRESS;
            }

            kernel_top_page_table[0] = 0; // reset id-map
            _invalidate_page(0);

            return {};
        }
    }

    hal_result init_sub_cores(void);
    hal_result init_sub_core(void);
    void       init_global_constructors(void);

    hal_result init_architecture_impl(a9n::word arch_info[])
    {
        init_global_constructors();

        for (auto i = 0; i < kernel::ARCH_INFO_MAX; i++)
        {
            a9n::kernel::utility::logger::printh(
                "arch_info: offset=%4d, value=0x%016llx\n",
                i,
                arch_info[i]
            );
        }

        return init_main_core(arch_info[0])
            .and_then(
                [&]() -> hal_result
                {
                    if constexpr (!kernel::SMP_ENABLED)
                    {
                        return {};
                    }

                    return init_sub_cores().or_else(
                        [](hal_error e) -> hal_result
                        {
                            if (e != hal_error::UNSUPPORTED)
                            {
                                return e;
                            }

                            a9n::kernel::utility::logger::printh("SMP is unsupported\n");
                            return {};
                        }
                    );
                }
            )
            .and_then(unmap_lower_memory_mapping);
    }

    extern "C"
    {
        extern uint8_t __init_constructors_start[];
        extern uint8_t __init_constructors_end[];
    }

    void init_global_constructors(void)
    {
        a9n::kernel::utility::logger::printh("Calling C++ global constructors ...\n");

        using constructor = void (*)(void);
        a9n::word constructor_count
            = (reinterpret_cast<a9n::word>(&__init_constructors_end)
               - reinterpret_cast<a9n::word>(&__init_constructors_end))
            / sizeof(constructor);
        constructor *constructors = reinterpret_cast<constructor *>(&__init_constructors_start);
        for (auto i = 0; i < constructor_count; i++)
        {
            constructors[i]();
        }
    }

    hal_result init_main_core(a9n::physical_address rsdp_address)
    {
        using a9n::kernel::utility::logger;

        a9n::hal::configure_local_variable(&a9n::kernel::cpu_local_variables[0]);

        // init *main* core (i.e., BSP)
        // enable cpu features
        return init_cpu_core()
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::error("Failed to initialize CPU");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    logger::printh("Initializing Local APIC ...\n");
                    return local_apic_core.init();
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    return local_apic_core.id().and_then(
                        [](uint8_t id) -> hal_result
                        {
                            arch_cpu_local_variables[kernel::BSP_ID].local_apic_id = id;
                            return {};
                        }
                    );
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    logger::printh("Initializing 8259 PIC ...\n");
                    init_pic(); // disable

                    return {};
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
                    logger::printh("Initializing IO APIC ...\n");
                    return io_apic_core.init(madt_base);
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("Initializing IMCR in IO APIC\n");
                    configure_imcr_to_apic(); // bsp
                    return {};
                }
            )
            .and_then(
                [](void) -> liba9n::result<fadt *, hal_error>
                {
                    logger::printh("Initializing ACPI ...\n");
                    return acpi_core.current_fadt();
                }
            )
            .and_then(
                [](fadt *fadt_base) -> hal_result
                {
                    logger::printh("Initializing ACPI PM Timer ...\n");
                    return acpi_pm_timer_core.init(fadt_base);
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("HAL error: %s\n", hal_error_to_string(e));
                    logger::error("Failed to initialize ACPI / APIC");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("Initializing Local APIC Timer ...\n");
                    return local_apic_timer_core.init();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("HAL error: %s\n", hal_error_to_string(e));
                    logger::error("Failed to initialize Local APIC Timer");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("Initializing syscall ...\n");
                    return init_syscall();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("HAL error: %s\n", hal_error_to_string(e));
                    logger::error("Failed to initialize syscall");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    // return interrupt_core.init();
                    a9n::kernel::utility::logger::printh("Initializing IDT handler ...\n");
                    return init_idt_handler();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("HAL error: %s\n", hal_error_to_string(e));
                    logger::error("failed to initialize interrupt");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    logger::printh("Reconfiguring serial port ...\n");
                    reconfigure_serial();
                    return {};
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("Loading vendor ID ...\n");
                    return try_get_vendor_id().and_then(
                        [](vendor_id id) -> hal_result
                        {
                            a9n::kernel::utility::logger::printh("  Vendor ID=%s\n", id.data());
                            return {};
                        }
                    );
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("Loading CPU name ...\n");
                    return try_get_cpu_name().and_then(
                        [](cpu_name name) -> hal_result
                        {
                            a9n::kernel::utility::logger::printh("  CPU name=%s\n", name.data());
                            return {};
                        }
                    );
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("Checking huge page support ...\n");
                    return try_get_cpuid(cpuid_leaf::EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS, 0)
                        .and_then(
                            [](cpuid_info info) -> hal_result
                            {
                                const bool is_2mib_page_supported = info.rdx & (1 << 29);
                                const bool is_1gib_page_supported = info.rdx & (1 << 26);

                                SUPPORT_2MiB_PAGE                 = is_2mib_page_supported;
                                SUPPORT_1GiB_PAGE                 = is_1gib_page_supported;

                                a9n::kernel::utility::logger::printh(
                                    "  2MiB page support=%s\n",
                                    is_2mib_page_supported ? "yes" : "no"
                                );
                                a9n::kernel::utility::logger::printh(
                                    "  1GiB page support=%s\n",
                                    is_1gib_page_supported ? "yes" : "no"
                                );

                                return {};
                            }
                        );
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    return enable_vmx()
                        // .and_then(run_test_vm)
                        .or_else(
                            [](hal_error e) -> hal_result
                            {
                                if (e != hal_error::UNSUPPORTED)
                                {
                                    return e;
                                }
                                a9n::kernel::utility::logger::printh(
                                    "HAL error: %s\n",
                                    hal_error_to_string(e)
                                );
                                a9n::kernel::utility::logger::printh("Virtualization is unsupported\n");

                                return {};
                            }
                        );
                }
            );
    }

    // source
    extern "C" uint8_t __boot_ap_trampoline_original_start[];
    extern "C" uint8_t __boot_ap_trampoline_original_end[];

    // destination
    extern "C" uint8_t __boot_ap_trampoline_start[];
    extern "C" uint8_t __boot_ap_trampoline_end[];

    // gdt
    extern "C" uint8_t __boot_ap_trampoline_gdtr[];
    extern "C" uint8_t __boot_ap_gdt_start[];
    extern "C" uint8_t __boot_ap_gdt_end[];

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

        a9n::physical_address destination_gdtr
            = reinterpret_cast<a9n::physical_address>(__boot_ap_trampoline_gdtr);

        a9n::physical_address source_gdt_start
            = reinterpret_cast<a9n::physical_address>(__boot_ap_gdt_start);
        a9n::physical_address source_gdt_end
            = reinterpret_cast<a9n::physical_address>(__boot_ap_gdt_end);
        a9n::word source_gdt_size = source_gdt_end - source_gdt_start;

        // copy trampolines!
        logger::printh(
            "Copying trampoline codes ... [0x%016llx - 0x%016llx) -> 0x%016llx\n",
            source_trampoline_start,
            (source_trampoline_start + boot_ap_trampoline_size),
            destination_trampoline_start
        );
        liba9n::std::memcpy(
            a9n::kernel::physical_to_virtual_pointer<void *>(destination_trampoline_start),
            a9n::kernel::physical_to_virtual_pointer<void *>(source_trampoline_start),
            boot_ap_trampoline_size
        );

        // make gdtr
        logger::printh("Making Global Descriptor Table (GDTR) for AP... (0x%016llx)\n", destination_gdtr);
        *a9n::kernel::physical_to_virtual_pointer<uint16_t>(destination_gdtr)
            = static_cast<uint16_t>(source_gdt_size - 1);
        *a9n::kernel::physical_to_virtual_pointer<uint64_t>(destination_gdtr + 2)
            = static_cast<uint64_t>(source_gdt_start);

        // get smp
        // return {};

        auto smp_info_result = create_smp_info();
        if (!smp_info_result)
        {
            return smp_info_result.unwrap_error();
        }

        auto detected_cpu_count = smp_info_result.unwrap()->enabled_ap_count;
        if (detected_cpu_count <= 1)
        {
            // single core
            return hal_error::UNSUPPORTED;
        }

        auto bsp_apic_id_result = local_apic_core.id();
        if (!bsp_apic_id_result)
        {
            return bsp_apic_id_result.unwrap_error();
        }

        a9n::word activated_cpu_count = 1;
        for (a9n::word i = 0;
             i < detected_cpu_count && activated_cpu_count < a9n::kernel::CPU_COUNT_MAX;
             i++)
        {
            auto local_apic_id = smp_info_result.unwrap()->local_apic_ids[i];
            if (local_apic_id == bsp_apic_id_result.unwrap())
            {
                continue;
            }
            if (local_apic_id > UINT8_MAX)
            {
                return hal_error::UNSUPPORTED;
            }

            logger::printh(
                "Starting core (core=%4d, Local APIC ID=0x%08lx) ...\n",
                activated_cpu_count,
                local_apic_id
            );
            auto result
                = ipi_init(local_apic_id)
                      .and_then(
                          [=](void) -> hal_result
                          {
                              // wait 10ms
                              return acpi_pm_timer_core.wait(10000);
                          }
                      )
                      .and_then(
                          [=](void) -> hal_result
                          {
                              // SIPI (1)
                              return ipi_startup(destination_trampoline_start, local_apic_id);
                          }
                      )
                      .and_then(
                          [=](void) -> hal_result
                          {
                              return acpi_pm_timer_core.wait(200);
                          }
                      )
                      .and_then(
                          [=](void) -> hal_result
                          {
                              // SIPI (2)
                              return ipi_startup(destination_trampoline_start, local_apic_id);
                          }
                      )
                      .and_then(
                          [=](void) -> hal_result
                          {
                              // wait 1ms
                              return acpi_pm_timer_core.wait(1000);
                          }
                      );
            if (!result)
            {
                return result.unwrap_error();
            }

            activated_cpu_count++;
        }

        expected_core_count = activated_cpu_count;

        logger::printh("AP successfully activated\n");

        return {};
    }

    // for smp
    extern "C" void x86_64_ap_entry(void)
    {
        kernel::utility::logger::printh("AP entry ...\n");

        auto result
            = try_allocate_core_number()
                  .and_then(
                      [](a9n::word core_number) -> hal_result
                      {
                          kernel::utility::logger::printh(
                              "Configuring CPU local variable [%04llx]\n",
                              core_number
                          );
                          return a9n::hal::configure_local_variable(
                              &a9n::kernel::cpu_local_variables[core_number]
                          );
                      }
                  )
                  .and_then(
                      [](void) -> hal_result
                      {
                          kernel::utility::logger::printh("Initializing core ...\n");
                          return init_sub_core();
                      }
                  );
        // .and_then(enable_vmx);
        if (!result)
        {
            atomic_store(&has_ap_boot_failed, 1);
            a9n::kernel::utility::logger::error("Can't configure AP");
            return;
        }

        auto local_variable_result = a9n::hal::current_local_variable();
        if (!local_variable_result)
        {
            atomic_store(&has_ap_boot_failed, 1);
            a9n::kernel::utility::logger::error("Can't retrieve AP local variable");
            return;
        }

        auto *local_variable = local_variable_result.unwrap();

        while (!atomic_load(&is_ap_runnable))
        {
            asm volatile("pause" ::: "memory");
        }

        auto kernel_result
            = local_variable->process_manager_core.init(local_variable->core_number)
                  .and_then(
                      [](void) -> kernel::kernel_result
                      {
                          return local_apic_timer_core.init_current_core().transform_error(
                              kernel::convert_hal_to_kernel_error
                          );
                      }
                  );
        if (!kernel_result)
        {
            atomic_store(&has_ap_boot_failed, 1);
            kernel::utility::logger::error("Can't initialize AP process manager");
            return;
        }

        kernel::kernel_result idle_result;
        {
            kernel::lock_guard guard(kernel::giant_lock);
            idle_result = local_variable->process_manager_core.switch_to_idle();
        }
        if (!idle_result)
        {
            atomic_store(&has_ap_boot_failed, 1);
            kernel::utility::logger::error("Can't switch AP to IDLE");
            return;
        }
        mark_core_booted();
        idle_loop();
    }

    hal_result init_sub_core(void)
    {
        using a9n::kernel::utility::logger;

        // long mode things
        return init_cpu_core()
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("HAL error: %s\n", hal_error_to_string(e));
                    logger::error("Failed to initialize CPU");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    logger::printh("Initializing Local APIC (AP) ...\n");
                    return local_apic_core.init();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("HAL error: %s\n", hal_error_to_string(e));
                    logger::error("Failed to initialize Local APIC (AP)");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    return current_local_variable().and_then(
                        [](kernel::cpu_local_variable *local_variable) -> hal_result
                        {
                            return local_apic_core.id().and_then(
                                [local_variable](uint8_t id) -> hal_result
                                {
                                    arch_cpu_local_variables[local_variable->core_number].local_apic_id
                                        = id;
                                    return {};
                                }
                            );
                        }
                    );
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    logger::printh("Initializing IDT handler (AP) ...\n");
                    return init_idt_handler();
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    a9n::kernel::utility::logger::printh("Initializing syscall (AP) ...\n");
                    return init_syscall();
                }
            )
            .or_else(
                [](hal_error e) -> hal_result
                {
                    logger::printh("HAL error: %s\n", hal_error_to_string(e));
                    logger::error("Failed to initialize syscall (AP)");
                    return e;
                }
            )
            .and_then(
                [](void) -> hal_result
                {
                    return enable_vmx()
                        // .and_then(run_test_vm)
                        .or_else(
                            [](hal_error e) -> hal_result
                            {
                                if (e != hal_error::UNSUPPORTED)
                                {
                                    return e;
                                }

                                a9n::kernel::utility::logger::printh(
                                    "HAL error: %s\n",
                                    hal_error_to_string(e)
                                );
                                a9n::kernel::utility::logger::printh("Virtualization is unsupported\n");
                                return {};
                            }
                        );
                }
            );
    };
}

namespace a9n::hal
{
    hal_result init_architecture(a9n::word arch_info[])
    {
        return x86_64::init_architecture_impl(arch_info);
    }
}
