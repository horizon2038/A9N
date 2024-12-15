#ifndef HAL_X86_64_ACPI_HPP
#define HAL_X86_64_ACPI_HPP

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    struct rsdp
    {
        // ACPI v1 field
        uint8_t  signature[8];
        uint8_t  checksum;
        uint8_t  oem_id[6];
        uint8_t  revision;
        uint32_t rsdt_address;

        // ACPI v2 field
        uint32_t length;
        uint64_t xsdt_address;
        uint8_t  extended_checksum;
        uint8_t  reserved[3];
    } __attribute__((packed));

    struct sdt_header
    {
        uint8_t  signature[4];
        uint32_t length;
        uint8_t  revision;
        uint8_t  checksum;
        uint8_t  oem_id[6];
        uint8_t  oem_table_id[8];
        uint32_t oem_revision;
        uint32_t creator_id;
        uint32_t creator_revision;

        bool validate_sdt_signature(const char *target_signature);
    } __attribute__((packed));

    struct xsdt
    {
      public:
        sdt_header header;

        uint32_t    count();
        sdt_header *search_sdt_header(uint32_t count);

      private:
        uint64_t *calculate_table_head();

    } __attribute__((packed));

    enum class generic_address_space_type : uint8_t
    {
        SYSTEM_MEMORY                                  = 0x00,
        SYSTEM_IO                                      = 0x01,
        PCI_CONFIGURATION_SPACE                        = 0x02,
        EMBEDDED_CONTROLLER                            = 0x03,
        SYSTEM_MANAGEMENT_BUS                          = 0x04,
        SYSTEM_CMOS                                    = 0x05,
        PCI_DEVICE_BAR_TARGET                          = 0x06,
        INTELLIGENT_PLATFORM_MANAGEMENT_INFRASTRUCTURE = 0x07,
        GENERAL_PURPOSE_IO                             = 0x08,
        GENERIC_SERIAL_BUS                             = 0x09,
        PLATFORM_COMMUNICATION_CHANNEL                 = 0x0A,
        PLATFORM_RUNTIME_MECHANISM                     = 0x0B,
        // 0x0C to 0x7E is reserved
        FUNCTIONAL_FIXED_HARDWARE = 0x7F,
        // 0x80 to 0xFF is oem defined
    };

    enum class generic_address_access_size : uint8_t
    {
        UNDEFINED     = 0x00,
        BYTE_1_ACCESS = 0x01,
        BYTE_2_ACCESS = 0x02,
        BYTE_4_ACCESS = 0x03,
        BYTE_8_ACCESS = 0x04,
    };

    struct generic_address_structure
    {
        generic_address_space_type  address_space;
        uint8_t                     bit_width;
        uint8_t                     bit_offset;
        generic_address_access_size access_size;
        uint64_t                    address;
    } __attribute__((packed));

    struct madt
    {
        sdt_header header;
        uint32_t   local_apic_address;
        uint32_t   flags;
    } __attribute__((packed));

    enum class madt_entry_type : uint8_t
    {
        LOCAL_APIC                    = 0x00,
        IO_APIC                       = 0x01,
        INTERRUPT_SOURCE_OVERRIDE     = 0x02,
        NON_MASKABLE_INTERRUPT_SOURCE = 0x03,
        LOCAL_APIC_NMI                = 0x04,
        LOCAL_APIC_ADDRESS_OVERRIDE   = 0x05,
        IO_SAPIC                      = 0x06,
        LOCAL_SAPIC                   = 0x07,
        PLATFORM_INTERRUPT_SOURCES    = 0x08,
        PROCESSOR_LOCAL_X2APIC        = 0x09,
        LOCAL_X2APIC_NMI              = 0x0a,
    };

    struct madt_entry_header
    {
        madt_entry_type type;
        uint8_t         length;
    } __attribute__((packed));

    struct madt_local_apic
    {
        madt_entry_type type;
        uint8_t         length;
        uint8_t         acpi_processor_id;
        uint8_t         apic_id;
        uint32_t        flags;
    } __attribute__((packed));

    struct madt_local_x2_apic
    {
        madt_entry_type type;
        uint8_t         length;
        uint16_t        reserved;
        uint32_t        apic_id;
        uint32_t        flags;
        uint32_t        acpi_processor_id;

    } __attribute__((packed));

    struct madt_io_apic
    {
        madt_entry_type type;
        uint8_t         length;
        uint8_t         io_apic_id;
        uint8_t         reserved;
        uint32_t        io_apic_address;
        uint32_t        global_system_interrupt_base;
    } __attribute__((packed));

    struct fadt
    {
        sdt_header header; // 36 byte
        uint32_t   firmware_control;
        uint32_t   dsdt;

        // ACPI v1 reserved.
        uint8_t reserved;

        uint8_t  preferred_power_management_profile;
        uint16_t sci_interrupt;
        uint32_t smi_commandport;
        uint8_t  acpi_enable;
        uint8_t  acpi_disable;
        uint8_t  s4bios_request;
        uint8_t  pstate_control;
        uint32_t pm1a_event_block;
        uint32_t pm1b_event_block;
        uint32_t pm1a_control_block;
        uint32_t pm1b_control_block;
        uint32_t pm2_control_block;
        uint32_t pm_timer_block;
        uint32_t gpe0_block;
        uint32_t gpe1_block;
        uint8_t  pm1_event_length;
        uint8_t  pm1_control_length;
        uint8_t  pm2_control_length;
        uint8_t  pm_timer_length;
        uint8_t  gpe0_length;
        uint8_t  gpe1_length;
        uint8_t  gpe1_base;
        uint8_t  cstate_control;
        uint16_t worst_c2_latency;
        uint16_t worst_c3_latency;
        uint16_t flush_size;
        uint16_t flush_stride;
        uint8_t  duty_offset;
        uint8_t  duty_width;
        uint8_t  day_alarm;
        uint8_t  month_alarm;
        uint8_t  century;

        // ACPI v1 reserved.
        uint16_t boot_architecture_flags;

        uint8_t  reserved_2;
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
        generic_address_structure x_pm1b_bcontrol_block;
        generic_address_structure x_pm2_control_block;
        generic_address_structure x_pm_timer_block;
        generic_address_structure x_gpe0_block;
        generic_address_structure x_gpe1_block;
    } __attribute__((packed));

    namespace acpi_region
    {
        static constexpr a9n::physical_address BIOS_MAIN_START = 0x000E0000;
        static constexpr a9n::physical_address BIOS_MAIN_END   = 0x00100000;
        static constexpr a9n::physical_address EBDA_SEGMENT    = 0x0000040E;
    }

    namespace acpi_magic
    {
        static constexpr char RSDP[8] = { 'R', 'S', 'D', ' ', 'P', 'T', 'R', ' ' };
    }

    namespace acpi_structures
    {
        inline static rsdp *rsdp_pointer;
        inline static xsdt *xsdt_pointer;
    }

    class acpi_configurator
    {
      public:
        void                                    init(a9n::virtual_address rsdp_address);
        liba9n::result<rsdp *, hal_error>       current_rsdp();
        liba9n::result<sdt_header *, hal_error> current_rsdt();
        liba9n::result<xsdt *, hal_error>       current_xsdt();
        liba9n::result<fadt *, hal_error>       current_fadt();
        liba9n::result<madt *, hal_error>       current_madt();
        liba9n::result<sdt_header *, hal_error> current_hpet();

      private:
        bool validate_rsdp(a9n::virtual_address rsdp_address);
        void print_rsdp_info(rsdp *target_rsdp);
        void print_sdt_header_info(sdt_header *header);

        rsdp       *rsdp_address;
        sdt_header *rsdt_address;
        xsdt       *xsdt_address;
        fadt       *fadt_address;
        madt       *madt_address;
        sdt_header *hpet_address;
    };

    inline acpi_configurator acpi_core {};

}

#endif
