#ifndef A9N_HAL_X86_64_SMP_HPP
#define A9N_HAL_X86_64_SMP_HPP

#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/platform/acpi.hpp>

#include <liba9n/common/not_null.hpp>
#include <liba9n/libcxx/array>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    struct smp_info
    {
        // without BSP
        a9n::word                                            enabled_ap_count { 0 };
        liba9n::std::array<a9n::word, kernel::CPU_COUNT_MAX> local_apic_ids;
    };

    inline smp_info smp_info_core {};

    inline liba9n::result<liba9n::not_null<smp_info>, hal_error> create_smp_info(void)
    {
        using a9n::kernel::utility::logger;

        auto result = acpi_core.current_madt().and_then(
            [&](madt *madt_base) -> hal_result
            {
                logger::printh("smp_info : configure from MADT\n");
                if (!madt_base)
                {
                    a9n::kernel::utility::logger::printk("MADT is empty\n");
                    return hal_error::ILLEGAL_ARGUMENT;
                }

                uint8_t *madt_entry_pointer = reinterpret_cast<uint8_t *>(madt_base) + sizeof(madt);
                uint8_t *end = reinterpret_cast<uint8_t *>(madt_base) + madt_base->header.length;

                while (madt_entry_pointer < end)
                {
                    madt_entry_header *entry = reinterpret_cast<madt_entry_header *>(madt_entry_pointer
                    );
                    logger::printh("madt entry : 0x%016llx\n", entry);

                    switch (entry->type)
                    {
                        case madt_entry_type::LOCAL_APIC :
                            {
                                auto *local_apic_entry = reinterpret_cast<madt_local_apic *>(entry);
                                if (local_apic_entry->flags & 1)
                                {
                                    smp_info_core.local_apic_ids[smp_info_core.enabled_ap_count]
                                        = local_apic_entry->apic_id;
                                    smp_info_core.enabled_ap_count++;
                                    logger::printh("madt type : Local APIC\n");
                                }

                                break;
                            }

                        case madt_entry_type::PROCESSOR_LOCAL_X2APIC :
                            {
                                auto *local_x2_apic_entry = reinterpret_cast<madt_local_x2_apic *>(entry
                                );

                                if (local_x2_apic_entry->flags & 1)
                                {
                                    smp_info_core.local_apic_ids[smp_info_core.enabled_ap_count]
                                        = local_x2_apic_entry->apic_id;
                                    smp_info_core.enabled_ap_count++;
                                    logger::printh("madt type : Local X2APIC\n");
                                }

                                break;
                            }

                        default :
                            break;
                    }

                    madt_entry_pointer += entry->length;
                }

                return {};
            }
        );

        if (!result)
        {
            return result.unwrap_error();
        }

        logger::printh("number of processor : %04d\n", smp_info_core.enabled_ap_count);

        return smp_info_core;
    }
}

#endif
