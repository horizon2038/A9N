#include <hal/x86_64/platform/acpi.hpp>

#include "kernel/memory/memory.hpp"
#include <hal/hal_result.hpp>
#include <hal/x86_64/arch/arch_types.hpp>
#include <kernel/types.hpp>

#include <kernel/utility/logger.hpp>
#include <liba9n/libc/string.hpp>

namespace a9n::hal::x86_64
{
    bool sdt_header::validate_sdt_signature(const char *target_signature)
    {
        return ((liba9n::std::memcmp(signature, target_signature, 4) == 0));
    }

    uint32_t xsdt::count()
    {
        return (header.length - sizeof(sdt_header)) / 8;
    }

    sdt_header *xsdt::search_sdt_header(uint32_t count)
    {
        a9n::virtual_address *sdt_address_table = calculate_table_head();
        return a9n::kernel::physical_to_virtual_pointer<sdt_header>(
            sdt_address_table[count]
        );
    }

    a9n::virtual_address *xsdt::calculate_table_head()
    {
        return a9n::kernel::physical_to_virtual_pointer<a9n::virtual_address>(
            reinterpret_cast<a9n::physical_address>(this) + (sizeof(uint8_t) * 36)
        );
    }

    void acpi_configurator::init(a9n::virtual_address initial_rsdp_address)
    {
        a9n::kernel::utility::logger::printk("init ACPI\n");

        if (!validate_rsdp(initial_rsdp_address))
        {
            a9n::kernel::utility::logger::printk("RSDP is invalid\n");
            return;
        }

        auto rsdp_pointer = reinterpret_cast<rsdp *>(initial_rsdp_address);
        print_rsdp_info(rsdp_pointer);

        xsdt *xsdt_pointer = a9n::kernel::physical_to_virtual_pointer<xsdt>(
            (rsdp_pointer->xsdt_address)
        );
        a9n::kernel::utility::logger::split();
        print_sdt_header_info(&xsdt_pointer->header);

        rsdt_address = a9n::kernel::physical_to_virtual_pointer<sdt_header>(
            rsdp_pointer->rsdt_address
        );
        xsdt_address = xsdt_pointer;

        for (auto i = 0; i < xsdt_pointer->count(); i++)
        {
            sdt_header *header = xsdt_pointer->search_sdt_header(i);
            print_sdt_header_info(header);

            if (header->validate_sdt_signature("FACP"))
            {
                fadt_address = reinterpret_cast<fadt *>(header);
                a9n::kernel::utility::logger::printk("configure FADT address\n");
                continue;
            }

            if (header->validate_sdt_signature("APIC"))
            {
                madt_address = reinterpret_cast<madt *>(header);
                a9n::kernel::utility::logger::printk("configure MADT address\n");
                continue;
            }

            if (header->validate_sdt_signature("HPET"))
            {
                hpet_address = header;
                a9n::kernel::utility::logger::printk("configure HPET address\n");
                continue;
            }
        }
    };

    bool acpi_configurator::validate_rsdp(a9n::virtual_address init_rsdp_address)
    {
        rsdp *rsdp_pointer = reinterpret_cast<rsdp *>(init_rsdp_address);
        return (
            liba9n::std::memcmp(rsdp_pointer->signature, acpi_magic::RSDP, 8) == 0
        );
    }

    void acpi_configurator::print_rsdp_info(rsdp *rsdp_pointer)
    {
        char signature[9];
        signature[8] = '\0';

        char oem_id[7];
        oem_id[6] = '\0';

        liba9n::std::memcpy(signature, rsdp_pointer->signature, 8);
        liba9n::std::memcpy(oem_id, rsdp_pointer->oem_id, 6);

        a9n::kernel::utility::logger::printk("signature\e[55G : %s\n", signature);
        a9n::kernel::utility::logger::printk(
            "checksum\e[55G : %d\n",
            rsdp_pointer->checksum
        );
        a9n::kernel::utility::logger::printk("oem_id\e[55G : %s\n", oem_id);
        a9n::kernel::utility::logger::printk(
            "revision\e[55G : %d\n",
            rsdp_pointer->revision
        );
        a9n::kernel::utility::logger::printk(
            "rsdt_address\e[55G : 0x%08lx\n",
            rsdp_pointer->rsdt_address
        );
        a9n::kernel::utility::logger::printk(
            "length\e[55G : 0x%08lx\n",
            rsdp_pointer->length
        );
        a9n::kernel::utility::logger::printk(
            "xsdt_address\e[55G : 0x%016llx\n",
            rsdp_pointer->xsdt_address
        );
        a9n::kernel::utility::logger::printk(
            "extended_checksum\e[55G : %d\n",
            rsdp_pointer->extended_checksum
        );
    }

    void acpi_configurator::print_sdt_header_info(sdt_header *header)
    {
        char fixed_signature[5];
        fixed_signature[4] = '\0';
        liba9n::std::memcpy(fixed_signature, header->signature, 4);

        char fixed_oem_id[7];
        fixed_oem_id[6] = '\0';
        liba9n::std::memcpy(fixed_oem_id, header->oem_id, 6);

        char fixed_oem_table_id[9];
        fixed_oem_table_id[8] = '\0';
        liba9n::std::memcpy(fixed_oem_table_id, header->oem_table_id, 8);

        a9n::kernel::utility::logger::printk("signature\e[55G : %s\n", fixed_signature);
        a9n::kernel::utility::logger::printk(
            "length\e[55G : 0x%08lx\n",
            header->length
        );
        a9n::kernel::utility::logger::printk("revision\e[55G : %d\n", header->revision);
        a9n::kernel::utility::logger::printk("checksum\e[55G : %d\n", header->checksum);
        a9n::kernel::utility::logger::printk("oem_id\e[55G : %s\n", fixed_oem_id);
        a9n::kernel::utility::logger::printk(
            "oem_table_id\e[55G : %s\n",
            fixed_oem_table_id
        );
        a9n::kernel::utility::logger::printk(
            "oem_revision\e[55G : 0x%08lx\n",
            header->oem_revision
        );
        a9n::kernel::utility::logger::printk(
            "creator_id\e[55G : 0x%08lx\n",
            header->creator_id
        );
        a9n::kernel::utility::logger::printk(
            "creator_revision\e[55G : 0x%08lx\n",
            header->creator_revision
        );
        a9n::kernel::utility::logger::split();
    };

    liba9n::result<rsdp *, hal_error> acpi_configurator::current_rsdp()
    {
        if (!rsdp_address)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        return rsdp_address;
    }

    liba9n::result<sdt_header *, hal_error> acpi_configurator::current_rsdt()
    {
        if (!rsdt_address)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        return rsdt_address;
    }

    liba9n::result<xsdt *, hal_error> acpi_configurator::current_xsdt()
    {
        if (!xsdt_address)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        return xsdt_address;
    }

    liba9n::result<fadt *, hal_error> acpi_configurator::current_fadt()
    {
        if (!fadt_address)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        return fadt_address;
    }

    liba9n::result<madt *, hal_error> acpi_configurator::current_madt()
    {
        if (!madt_address)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        return madt_address;
    }

    liba9n::result<sdt_header *, hal_error> acpi_configurator::current_hpet()
    {
        if (!hpet_address)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        return hpet_address;
    }
}
