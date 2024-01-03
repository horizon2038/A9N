#ifndef HAL_X86_64_ACPI_HPP
#define HAL_X86_64_ACPI_HPP

#include <library/common/types.hpp>

namespace hal::x86_64
{
    class acpi_configurator
    {
      public:
        acpi_configurator();
        ~acpi_configurator();

        void init_acpi();
        common::virtual_address find_rsdp();

      private:
        common::virtual_pointer find_rsdp_range(
            common::virtual_address start_address,
            common::virtual_address end_address
        );
    };

    struct rsdp
    {
        uint8_t signature[8];
        uint16_t checksum;
        uint8_t oem_id[6];
        uint8_t revision;
        uint32_t rsdt_address;
        uint32_t length;
        uint8_t xsdt_address[6];
        uint8_t extended_checksum;
        uint8_t reserved[3];
    } __attribute__((packed));

    namespace ACPI_REGION
    {
        constexpr static common::physical_address BIOS_MAIN_START = 0x000E0000;
        constexpr static common::physical_address BIOS_MAIN_END = 0x00100000;
        constexpr static common::physical_address EBDA_SEGMENT = 0x0000040E;
    }
    namespace ACPI_MAGIC
    {
        constexpr static char RSDP[]
            = { 'R', 'S', 'D', ' ', 'P', 'T', 'R', ' ', 0 };
    }
}

#endif
