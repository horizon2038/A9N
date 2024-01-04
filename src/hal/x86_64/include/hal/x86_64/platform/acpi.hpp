#ifndef HAL_X86_64_ACPI_HPP
#define HAL_X86_64_ACPI_HPP

#include <library/common/types.hpp>

namespace hal::x86_64
{
    struct rsdp
    {
        // ACPI v1 field
        uint8_t signature[8];
        uint8_t checksum;
        uint8_t oem_id[6];
        uint8_t revision;
        uint32_t rsdt_address;

        // ACPI V2 field
        uint32_t length;
        uint64_t xsdt_address;
        uint8_t extended_checksum;
        uint8_t reserved[3];
    } __attribute__((packed));

    struct sdt_header
    {
        uint8_t signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        uint8_t oem_id[6];
        uint8_t oem_table_id[8];
        uint32_t oem_regision;
        uint32_t creatoe_id;
        uint32_t creator_revision;
    } __attribute__((packed));

    struct xsdt
    {
        sdt_header header;
        uint64_t *other_sdt;
    } __attribute__((packed));

    namespace ACPI_REGION
    {
        constexpr static common::physical_address BIOS_MAIN_START = 0x000E0000;
        constexpr static common::physical_address BIOS_MAIN_END = 0x00100000;
        constexpr static common::physical_address EBDA_SEGMENT = 0x0000040E;
    }
    namespace ACPI_MAGIC
    {
        constexpr static char RSDP[8]
            = { 'R', 'S', 'D', ' ', 'P', 'T', 'R', ' ' };
    }

    namespace ACPI_ADDRESS
    {
        inline static common::virtual_address rsdp_address;
        inline static common::virtual_address xsdt_address;
        inline static common::virtual_address hpet_address;
    }

    class acpi_configurator
    {
      public:
        acpi_configurator();
        ~acpi_configurator();

        void init_acpi(common::virtual_address rsdp_address);

      private:
        bool validate_rsdp(common::virtual_address rsdp_address);
        void print_rsdp_info(rsdp *target_rsdp);
        void print_xsdt_info(xsdt *target_xsdt);
    };

}

#endif
