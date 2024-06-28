#include <liba9n/common/types.hpp>
#include <hal/x86_64/platform/acpi.hpp>

#include <hal/x86_64/arch/arch_types.hpp>
#include <liba9n/libc/string.hpp>
#include <kernel/utility/logger.hpp>

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
        a9n::virtual_address  sdt_virtual_address
            = convert_physical_to_virtual_address(sdt_address_table[count]);
        return reinterpret_cast<sdt_header *>(sdt_virtual_address);
    }

    a9n::virtual_address *xsdt::calculate_table_head()
    {
        return reinterpret_cast<a9n::virtual_address *>(
            convert_physical_to_virtual_address(
                reinterpret_cast<a9n::physical_address>(this)
                + (sizeof(uint8_t) * 36)
            )
        );
    }

    acpi_configurator::acpi_configurator() {};
    acpi_configurator::~acpi_configurator() {};

    void acpi_configurator::init_acpi(a9n::virtual_address rsdp_address)
    {
        a9n::kernel::utility::logger::printk("init acpi\n");
        if (!validate_rsdp(rsdp_address))
        {
            a9n::kernel::utility::logger::printk("rsdp is invalid\n");
            return;
        }
        a9n::kernel::utility::logger::printk("rsdp is valid\n");

        auto rsdp_pointer = reinterpret_cast<rsdp *>(rsdp_address);
        print_rsdp_info(rsdp_pointer);

        xsdt *xsdt_pointer = reinterpret_cast<xsdt *>(
            convert_physical_to_virtual_address(rsdp_pointer->xsdt_address)
        );
        a9n::kernel::utility::logger::split();
        print_sdt_header_info(&xsdt_pointer->header);

        for (auto i = 0; i < xsdt_pointer->count(); i++)
        {
            sdt_header *header = xsdt_pointer->search_sdt_header(i);
            print_sdt_header_info(header);
            if (header->validate_sdt_signature("FACP"))
            {
                a9n::kernel::utility::logger::printk("\e[32mFACP found!\e[0m\n"
                );
                a9n::kernel::utility::logger::split();
            }
        }
    };

    bool acpi_configurator::validate_rsdp(a9n::virtual_address rsdp_address)
    {
        rsdp *rsdp_pointer = reinterpret_cast<rsdp *>(rsdp_address);
        return (
            liba9n::std::memcmp(rsdp_pointer->signature, ACPI_MAGIC::RSDP, 8)
            == 0
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

        a9n::kernel::utility::logger::printk(
            "signature\e[55G : %s\n",
            signature
        );
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

        a9n::kernel::utility::logger::printk(
            "signature\e[55G : %s\n",
            fixed_signature
        );
        a9n::kernel::utility::logger::printk(
            "length\e[55G : 0x%08lx\n",
            header->length
        );
        a9n::kernel::utility::logger::printk(
            "revision\e[55G : %d\n",
            header->revision
        );
        a9n::kernel::utility::logger::printk(
            "checksum\e[55G : %d\n",
            header->checksum
        );
        a9n::kernel::utility::logger::printk(
            "oem_id\e[55G : %s\n",
            fixed_oem_id
        );
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

}
