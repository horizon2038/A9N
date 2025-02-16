#ifndef A9N_HAL_X86_64_VMCS_FIELD_HPP
#define A9N_HAL_X86_64_VMCS_FIELD_HPP

#include <kernel/types.hpp>
#include <liba9n/common/enum.hpp>

namespace a9n::hal::x86_64
{
    // VMCS field encoding
    // cf., Intel SDM vol. 3C 25-30, table 25-21, "Structure of VMCS Component Encoding"
    //      (combined : page 3966)
    // +-------------+-------+-------+----+-------+-------+
    // |      0      |  1-9  | 10-11 | 12 | 13-14 | 15-31 |
    // +-------------+-------+-------+----+-------+-------+
    // | access type | index | type  |  - | width |     - |
    // +-------------+-------+-------+----+-------+-------+

    enum class vmcs_access_type : bool
    {
        FULL = false,
        HIGH = true,
    };

    enum class vmcs_type : uint8_t
    {
        CONTROL             = 0,
        VM_EXIT_INFORMATION = 1,
        GUEST_STATE         = 2,
        HOST_STATE          = 3
    };

    enum class vmcs_width : uint8_t
    {
        BIT_16        = 0,
        BIT_64        = 1,
        BIT_32        = 2,
        NATURAL_WIDTH = 3,
    };

    struct vmcs_field
    {
        uint32_t all { 0 };

        constexpr vmcs_field(vmcs_access_type access_type, uint16_t index, vmcs_type type, vmcs_width width)
        {
            configure_access_type(access_type);
            configure_index(index);
            configure_type(type);
            configure_width(width);
            clear_reserved_section();
        }

        constexpr explicit operator uint32_t() const
        {
            return all;
        }

        constexpr void configure_access_type(vmcs_access_type access_type)
        {
            all |= liba9n::enum_cast(access_type);
        }

        constexpr void configure_index(uint16_t index)
        {
            index &= ((1 << 9) - 1);
            all   |= index << 1;
        }

        constexpr void configure_type(vmcs_type type)
        {
            all |= (static_cast<uint32_t>(liba9n::enum_cast(type)) << 10);
        }

        constexpr void configure_width(vmcs_width width)
        {
            all |= (static_cast<uint32_t>(liba9n::enum_cast(width)) << 13);
        }

        constexpr void clear_reserved_section(void)
        {
            // zero
            uint32_t reserved_1  = (static_cast<uint32_t>(1) << 12);
            uint32_t reserved_2  = ((static_cast<uint32_t>(1) << 17) - 1) << 15;

            all                 &= ~(reserved_1 | reserved_2);
        }
    };

    // VMCS field encodings
    // cf., Intel SDM vol. 3D [B-1]-[B-11], "FIELD ENCODING IN VMCS"
    //      (combined : page 4491)
    namespace vmcs_fields
    {
        /* ===== 16-bit section ===== */
        // control fields
        inline constexpr vmcs_field VIRTUAL_PROCESSOR_IDENTIFIER
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::CONTROL, vmcs_width::BIT_16);
        inline constexpr vmcs_field POSTED_INTERRUPT_NOTIFICATION_VECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::CONTROL, vmcs_width::BIT_16);
        inline constexpr vmcs_field EPTP_INDEX
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::CONTROL, vmcs_width::BIT_16);
        inline constexpr vmcs_field HLAT_PREFIX_SIZE
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::CONTROL, vmcs_width::BIT_16);
        inline constexpr vmcs_field LAST_PID_POINTER_INDEX
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::CONTROL, vmcs_width::BIT_16);

        // guest-state fields
        inline constexpr vmcs_field GUEST_ES_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_CS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_SS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_DS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_FS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_GS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000101, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_LDTR_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000110, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_TR_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000111, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_INTERRUPT_STATUS
            = vmcs_field(vmcs_access_type::FULL, 0b00001000, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field PML_INDEX
            = vmcs_field(vmcs_access_type::FULL, 0b00001001, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field GUEST_UINV
            = vmcs_field(vmcs_access_type::FULL, 0b00001010, vmcs_type::GUEST_STATE, vmcs_width::BIT_16);

        // host-state fields
        inline constexpr vmcs_field HOST_ES_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::HOST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field HOST_CS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::HOST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field HOST_SS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::HOST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field HOST_DS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::HOST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field HOST_FS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::HOST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field HOST_GS_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000101, vmcs_type::HOST_STATE, vmcs_width::BIT_16);
        inline constexpr vmcs_field HOST_TR_SELECTOR
            = vmcs_field(vmcs_access_type::FULL, 0b00000110, vmcs_type::HOST_STATE, vmcs_width::BIT_16);

        /* ===== 64-bit section ===== */
        // control fields
        inline constexpr vmcs_field ADDRESS_OF_IO_BITMAP_A
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field ADDRESS_OF_IO_BITMAP_B
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field ADDRESS_OF_MSR_BITMAPS
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VM_EXIT_MSR_STORE_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VM_EXIT_MSR_LOAD_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VM_ENTRY_MSR_LOAD_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00000101, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field EXECUTIVE_VMCS_POINTER
            = vmcs_field(vmcs_access_type::FULL, 0b00000110, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field PML_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00000111, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field TSC_OFFSET
            = vmcs_field(vmcs_access_type::FULL, 0b00001000, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VIRTUAL_APIC_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00001001, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field APIC_ACCESS_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00001010, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field POSTED_INTERRUPT_DESCRIPTOR_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00001011, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VM_FUNCTION_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00001100, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field EPT_POINTER
            = vmcs_field(vmcs_access_type::FULL, 0b00001101, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field EOI_EXIT_BITMAP_0
            = vmcs_field(vmcs_access_type::FULL, 0b00001110, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field EOI_EXIT_BITMAP_1
            = vmcs_field(vmcs_access_type::FULL, 0b00001111, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field EOI_EXIT_BITMAP_2
            = vmcs_field(vmcs_access_type::FULL, 0b00010000, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field EOI_EXIT_BITMAP_3
            = vmcs_field(vmcs_access_type::FULL, 0b00010001, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field EPTP_LIST_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00010010, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VMREAD_BITMAP_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00010011, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VMWRITE_BITMAP_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00010100, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field VIRTUALIZATION_EXCEPTION_INFORMATION_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00010101, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field XSS_EXITING_BITMAP
            = vmcs_field(vmcs_access_type::FULL, 0b00010110, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field ENCLS_EXITING_BITMAP
            = vmcs_field(vmcs_access_type::FULL, 0b00010111, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field SUB_PAGE_PERMISSION_TABLE_POINTER
            = vmcs_field(vmcs_access_type::FULL, 0b00011000, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field TSC_MULTIPLIER
            = vmcs_field(vmcs_access_type::FULL, 0b00011001, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field TERTIARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00011010, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field ENCLV_EXITING_BITMAP
            = vmcs_field(vmcs_access_type::FULL, 0b00011011, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field LOW_PASID_DIRECTORY_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00011100, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field HIGH_PASID_DIRECTORY_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00011101, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field SHARED_EPT_POINTER
            = vmcs_field(vmcs_access_type::FULL, 0b00011110, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field PCONFIG_EXITING_BITMAP
            = vmcs_field(vmcs_access_type::FULL, 0b00011111, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field HYPERVISOR_MANAGED_LINEAR_ADDRESS_TRANSLATION_POINTER
            = vmcs_field(vmcs_access_type::FULL, 0b00100000, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field PID_POINTER_TABLE_ADDRESS
            = vmcs_field(vmcs_access_type::FULL, 0b00100001, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field SECONDARY_VM_EXIT_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00100010, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field IA32_SPEC_CTRL_MASK
            = vmcs_field(vmcs_access_type::FULL, 0b00100101, vmcs_type::CONTROL, vmcs_width::BIT_64);
        inline constexpr vmcs_field IA32_SPEC_CTRL_SHADOW
            = vmcs_field(vmcs_access_type::FULL, 0b00100110, vmcs_type::CONTROL, vmcs_width::BIT_64);

        // read-only data field
        inline constexpr vmcs_field GUEST_PHYSICAL_ADDRESS = vmcs_field(
            vmcs_access_type::FULL,
            0b00000000,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_64
        );

        // guest-state fields
        inline constexpr vmcs_field VMCS_LINK_POINTER
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field VMCS_LINK_POINTER_HIGH
            = vmcs_field(vmcs_access_type::HIGH, 0b00000000, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_DEBUGCTL
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_PAT
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_EFER
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_PERF_GLOBAL_CTRL
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_PDPTE0
            = vmcs_field(vmcs_access_type::FULL, 0b00000101, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_PDPTE1
            = vmcs_field(vmcs_access_type::FULL, 0b00000110, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_PDPTE2
            = vmcs_field(vmcs_access_type::FULL, 0b00000111, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_PDPTE3
            = vmcs_field(vmcs_access_type::FULL, 0b00001000, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_BNDCFGS
            = vmcs_field(vmcs_access_type::FULL, 0b00001001, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_RTIT_CTL
            = vmcs_field(vmcs_access_type::FULL, 0b00001010, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_LBR_CTL
            = vmcs_field(vmcs_access_type::FULL, 0b00001011, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field GUEST_IA32_PKRS
            = vmcs_field(vmcs_access_type::FULL, 0b00001100, vmcs_type::GUEST_STATE, vmcs_width::BIT_64);

        // host-state fields
        inline constexpr vmcs_field HOST_IA32_PAT
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::HOST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field HOST_IA32_EFER
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::HOST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field HOST_IA32_PERF_GLOBAL_CTRL
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::HOST_STATE, vmcs_width::BIT_64);
        inline constexpr vmcs_field HOST_IA32_PKRS
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::HOST_STATE, vmcs_width::BIT_64);

        /* ===== 32-bit section ===== */
        // control fields
        inline constexpr vmcs_field PIN_BASED_VM_EXECUTION_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field PRIMARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field EXCEPTION_BITMAP
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field PAGE_FAULT_ERROR_CODE_MASK
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field PAGE_FAULT_ERROR_CODE_MATCH
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field CR3_TARGET_COUNT
            = vmcs_field(vmcs_access_type::FULL, 0b00000101, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field PRIMARY_VM_EXIT_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00000110, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field VM_EXIT_MSR_STORE_COUNT
            = vmcs_field(vmcs_access_type::FULL, 0b00000111, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field VM_EXIT_MSR_LOAD_COUNT
            = vmcs_field(vmcs_access_type::FULL, 0b00001000, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field VM_ENTRY_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00001001, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field VM_ENTRY_MSR_LOAD_COUNT
            = vmcs_field(vmcs_access_type::FULL, 0b00001010, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field VM_ENTRY_INTERRUPTION_INFORMATION_FIELD
            = vmcs_field(vmcs_access_type::FULL, 0b00001011, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field VM_ENTRY_EXCEPTION_ERROR_CODE
            = vmcs_field(vmcs_access_type::FULL, 0b00001100, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field VM_ENTRY_INSTRUCTION_LENGTH
            = vmcs_field(vmcs_access_type::FULL, 0b00001101, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field TPR_THRESHOLD
            = vmcs_field(vmcs_access_type::FULL, 0b00001110, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS
            = vmcs_field(vmcs_access_type::FULL, 0b00001111, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field PLE_GAP
            = vmcs_field(vmcs_access_type::FULL, 0b00010000, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field PLE_WINDOW
            = vmcs_field(vmcs_access_type::FULL, 0b00010001, vmcs_type::CONTROL, vmcs_width::BIT_32);
        inline constexpr vmcs_field INSTRUCTION_TIMEOUT_CONTROL
            = vmcs_field(vmcs_access_type::FULL, 0b00010010, vmcs_type::CONTROL, vmcs_width::BIT_32);

        // read-only data field
        inline constexpr vmcs_field VM_INSTRUCTION_ERROR = vmcs_field(
            vmcs_access_type::FULL,
            0b00000000,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );
        inline constexpr vmcs_field EXIT_REASON = vmcs_field(
            vmcs_access_type::FULL,
            0b00000001,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );
        inline constexpr vmcs_field VM_EXIT_INTERRUPTION_INFORMATION = vmcs_field(
            vmcs_access_type::FULL,
            0b00000010,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );
        inline constexpr vmcs_field VM_EXIT_INTERRUPTION_ERROR_CODE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000011,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );
        inline constexpr vmcs_field IDT_VECTORING_INFORMATION_FIELD = vmcs_field(
            vmcs_access_type::FULL,
            0b00000100,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );
        inline constexpr vmcs_field IDT_VECTORING_ERROR_CODE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000101,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );
        inline constexpr vmcs_field VM_EXIT_INSTRUCTION_LENGTH = vmcs_field(
            vmcs_access_type::FULL,
            0b00000110,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );
        inline constexpr vmcs_field VM_EXIT_INSTRUCTION_INFORMATION = vmcs_field(
            vmcs_access_type::FULL,
            0b00000111,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::BIT_32
        );

        // guest-state fields
        inline constexpr vmcs_field GUEST_ES_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_CS_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_SS_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_DS_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_FS_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_GS_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000101, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_LDTR_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000110, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_TR_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00000111, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_GDTR_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00001000, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_IDTR_LIMIT
            = vmcs_field(vmcs_access_type::FULL, 0b00001001, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_ES_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00001010, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_CS_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00001011, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_SS_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00001100, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_DS_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00001101, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_FS_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00001110, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_GS_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00001111, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_LDTR_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00010000, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_TR_ACCESS_RIGHTS
            = vmcs_field(vmcs_access_type::FULL, 0b00010001, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_INTERRUPTIBILITY_STATE
            = vmcs_field(vmcs_access_type::FULL, 0b00010010, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_ACTIVITY_STATE
            = vmcs_field(vmcs_access_type::FULL, 0b00010011, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_SMBASE
            = vmcs_field(vmcs_access_type::FULL, 0b00010100, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field GUEST_IA32_SYSENTER_CS
            = vmcs_field(vmcs_access_type::FULL, 0b00010101, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);
        inline constexpr vmcs_field VMX_PREEMPTION_TIMER_VALUE
            = vmcs_field(vmcs_access_type::FULL, 0b00010111, vmcs_type::GUEST_STATE, vmcs_width::BIT_32);

        // host-state field
        inline constexpr vmcs_field HOST_IA32_SYSENTER_CS
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::HOST_STATE, vmcs_width::BIT_32);

        /* ===== natural-width section ===== */
        // control fields
        inline constexpr vmcs_field CR0_GUEST_HOST_MASK
            = vmcs_field(vmcs_access_type::FULL, 0b00000000, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);
        inline constexpr vmcs_field CR4_GUEST_HOST_MASK
            = vmcs_field(vmcs_access_type::FULL, 0b00000001, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);
        inline constexpr vmcs_field CR0_READ_SHADOW
            = vmcs_field(vmcs_access_type::FULL, 0b00000010, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);
        inline constexpr vmcs_field CR4_READ_SHADOW
            = vmcs_field(vmcs_access_type::FULL, 0b00000011, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);
        inline constexpr vmcs_field CR3_TARGET_VALUE_0
            = vmcs_field(vmcs_access_type::FULL, 0b00000100, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);
        inline constexpr vmcs_field CR3_TARGET_VALUE_1
            = vmcs_field(vmcs_access_type::FULL, 0b00000101, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);
        inline constexpr vmcs_field CR3_TARGET_VALUE_2
            = vmcs_field(vmcs_access_type::FULL, 0b00000110, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);
        inline constexpr vmcs_field CR3_TARGET_VALUE_3
            = vmcs_field(vmcs_access_type::FULL, 0b00000111, vmcs_type::CONTROL, vmcs_width::NATURAL_WIDTH);

        // read-only data field
        inline constexpr vmcs_field EXIT_QUALIFICATION = vmcs_field(
            vmcs_access_type::FULL,
            0b00000000,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field IO_RCX = vmcs_field(
            vmcs_access_type::FULL,
            0b00000001,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field IO_RSI = vmcs_field(
            vmcs_access_type::FULL,
            0b00000010,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field IO_RDI = vmcs_field(
            vmcs_access_type::FULL,
            0b00000011,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field IO_RIP = vmcs_field(
            vmcs_access_type::FULL,
            0b00000100,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_LINEAR_ADDRESS = vmcs_field(
            vmcs_access_type::FULL,
            0b00000101,
            vmcs_type::VM_EXIT_INFORMATION,
            vmcs_width::NATURAL_WIDTH
        );

        // guest-state fields
        inline constexpr vmcs_field GUEST_CR0 = vmcs_field(
            vmcs_access_type::FULL,
            0b00000000,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_CR3 = vmcs_field(
            vmcs_access_type::FULL,
            0b00000001,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_CR4 = vmcs_field(
            vmcs_access_type::FULL,
            0b00000010,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_ES_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000011,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_CS_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000100,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_SS_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000101,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_DS_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000110,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_FS_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000111,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_GS_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00001000,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_LDTR_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00001001,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_TR_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00001010,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_GDTR_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00001011,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_IDTR_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00001100,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_DR7 = vmcs_field(
            vmcs_access_type::FULL,
            0b00001101,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_RSP = vmcs_field(
            vmcs_access_type::FULL,
            0b00001110,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_RIP = vmcs_field(
            vmcs_access_type::FULL,
            0b00001111,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_RFLAGS = vmcs_field(
            vmcs_access_type::FULL,
            0b00010000,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_PENDING_DEBUG_EXCEPTIONS = vmcs_field(
            vmcs_access_type::FULL,
            0b00010001,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_IA32_SYSENTER_ESP = vmcs_field(
            vmcs_access_type::FULL,
            0b00010010,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_IA32_SYSENTER_EIP = vmcs_field(
            vmcs_access_type::FULL,
            0b00010011,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_IA32_S_CET = vmcs_field(
            vmcs_access_type::FULL,
            0b00010100,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_SSP = vmcs_field(
            vmcs_access_type::FULL,
            0b00010101,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field GUEST_IA32_INTERRUPT_SSP_TABLE_ADDR = vmcs_field(
            vmcs_access_type::FULL,
            0b00010110,
            vmcs_type::GUEST_STATE,
            vmcs_width::NATURAL_WIDTH
        );

        // host-state fields
        inline constexpr vmcs_field HOST_CR0 = vmcs_field(
            vmcs_access_type::FULL,
            0b00000000,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_CR3 = vmcs_field(
            vmcs_access_type::FULL,
            0b00000001,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_CR4 = vmcs_field(
            vmcs_access_type::FULL,
            0b00000010,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_FS_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000011,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_GS_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000100,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_TR_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000101,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_GDTR_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000110,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_IDTR_BASE = vmcs_field(
            vmcs_access_type::FULL,
            0b00000111,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_IA32_SYSENTER_ESP = vmcs_field(
            vmcs_access_type::FULL,
            0b00001000,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_IA32_SYSENTER_EIP = vmcs_field(
            vmcs_access_type::FULL,
            0b00001001,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_RSP = vmcs_field(
            vmcs_access_type::FULL,
            0b00001010,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_RIP = vmcs_field(
            vmcs_access_type::FULL,
            0b00001011,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_IA32_S_CET = vmcs_field(
            vmcs_access_type::FULL,
            0b00001100,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_SSP = vmcs_field(
            vmcs_access_type::FULL,
            0b00001101,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
        inline constexpr vmcs_field HOST_IA32_INTERRUPT_SSP_TABLE_ADDR = vmcs_field(
            vmcs_access_type::FULL,
            0b00001110,
            vmcs_type::HOST_STATE,
            vmcs_width::NATURAL_WIDTH
        );
    }

}

#endif
