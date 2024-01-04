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

        // ACPI v2 field
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
        uint32_t creator_id;
        uint32_t creator_revision;
    } __attribute__((packed));

    struct xsdt
    {
        sdt_header header;
        uint64_t *other_sdt;
    } __attribute__((packed));

    struct generic_address_structure
    {
        uint8_t address_space;
        uint8_t bit_width;
        uint8_t bit_offset;
        uint8_t access_size;
        uint64_t address;
    } __attribute__((packed));

    struct fadt
    {
        sdt_header header;
        uint32_t firmware_control;
        uint32_t dsdt;

        // ACPI v1 reserved.
        uint8_t reserved;

        uint8_t preferred_power_management_profile;
        uint16_t sci_interrupt;
        uint32_t smi_commandport;
        uint8_t acpi_enable;
        uint8_t acpi_disable;
        uint8_t s4bios_request;
        uint8_t pstate_control;
        uint32_t pm1a_event_block;
        uint32_t pm1b_event_block;
        uint32_t pm1a_control_block;
        uint32_t pm1b_control_block;
        uint32_t pm2_control_block;
        uint32_t pm_timer_block;
        uint32_t gpe0_block;
        uint32_t gpe1_block;
        uint8_t pm1_event_length;
        uint8_t pm1_control_length;
        uint8_t pm2_control_length;
        uint8_t pm_timer_length;
        uint8_t gpe0_length;
        uint8_t gpe1_length;
        uint8_t gpe1_base;
        uint8_t cstate_control;
        uint16_t worst_c2_latency;
        uint16_t worst_c3_latency;
        uint16_t flush_size;
        uint16_t flush_stride;
        uint8_t duty_offset;
        uint8_t duty_width;
        uint8_t day_alarm;
        uint8_t month_alarm;
        uint8_t century;

        // ACPI v1 reserved.
        uint16_t boot_architecture_flags;

        uint8_t reserved_2;
        uint32_t flags;

        generic_address_structure reset_register;

        uint8_t reset_value;
        uint8_t reserved_3[3];

        // ACPI v2 field
        uint64_t x_firmware_control;
        uint64_t x_dsdt;

        generic_address_structure x_pm1a_event_block;
        generic_address_structure x_pm1b_event_block;
        generic_address_structure x_pm1a_control_block;
        generic_address_structure x_pm1_bcontrol_block;
        generic_address_structure x_pm2_control_block;
        generic_address_structure x_pm_timer_block;
        generic_address_structure x_gpe0_block;
        generic_address_structure x_gpe1_block;
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
