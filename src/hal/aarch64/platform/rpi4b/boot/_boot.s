.section .boot.entry, "ax"
.align 12

.global _boot_kernel_gateway
.type _boot_kernel_gateway, %function

.extern __bss_start
.extern __bss_end
.extern __boot_stack_end
.extern __kernel_l0
.extern __kernel_l1
.extern __kernel_l2
.extern aarch64_prepare_boot_protocol
.extern kernel_entry

_boot_kernel_gateway:
    // 64-byte little-endian Linux AArch64 Image header consumed by U-Boot booti.
    b .Lboot_start
    nop
    .quad 0x80000
    .quad __kernel_end - 0x00080000
    .quad 0
    .quad 0
    .quad 0
    .quad 0
    .long 0x644d5241
    .long 0

.Lboot_start:
    msr daifset, #0xf
    mov x19, x0

    ldr x20, =__boot_stack_end
    mov sp, x20

    mrs x1, CurrentEL
    cmp x1, #0x8
    b.eq .Ldrop_from_el2
    cmp x1, #0x4
    b.eq .Lat_el1

.Lunsupported_el:
    wfe
    b .Lunsupported_el

.Ldrop_from_el2:
    msr sp_el1, x20
    mov x1, #(1 << 31)
    msr hcr_el2, x1
    msr cptr_el2, xzr
    mov x1, #3
    msr cnthctl_el2, x1
    msr cntvoff_el2, xzr
    adr x1, .Lat_el1
    msr elr_el2, x1
    mov x1, #0x3c5
    msr spsr_el2, x1
    eret

.Lat_el1:
    mov sp, x20

    // Clear all zero-initialized kernel storage before constructing tables.
    ldr x1, =__bss_start
    ldr x2, =__bss_end
.Lclear_bss:
    cmp x1, x2
    b.hs .Lbuild_page_tables
    stp xzr, xzr, [x1], #16
    b .Lclear_bss

.Lbuild_page_tables:
    ldr x20, =__kernel_l0
    ldr x21, =__kernel_l1
    ldr x22, =__kernel_l2

    orr x1, x21, #3
    str x1, [x20]
    str x1, [x20, #(256 * 8)]

    mov x2, #0
.Lfill_l1:
    cmp x2, #16
    b.hs .Lfill_l2
    lsl x3, x2, #12
    add x3, x22, x3
    orr x3, x3, #3
    str x3, [x21, x2, lsl #3]
    add x2, x2, #1
    b .Lfill_l1

.Lfill_l2:
    mov x2, #0
    mov x3, #0
    movz x8, #0xfc00, lsl #16
    mov x10, #0x100000000
    movz x9, #0x60, lsl #48
.Lfill_l2_entry:
    cmp x2, #(16 * 512)
    b.hs .Lenable_mmu
    cmp x3, x8
    b.lo .Lnormal_memory
    cmp x3, x10
    b.hs .Lnormal_memory

    // BCM2711 peripheral and ARM-local windows: Device-nGnRE, PXN.
    mov x4, x3
    mov x5, #0x405
    orr x4, x4, x5
    orr x4, x4, x9
    b .Lstore_l2_entry

.Lnormal_memory:
    // SDRAM: Normal WBWA, inner-shareable, accessed 2 MiB block.
    mov x4, x3
    mov x5, #0x701
    orr x4, x4, x5

.Lstore_l2_entry:
    str x4, [x22, x2, lsl #3]
    add x3, x3, #0x200000
    add x2, x2, #1
    b .Lfill_l2_entry

.Lenable_mmu:
    // Attr0: Normal WBWA. Attr1: Device-nGnRE.
    mov x1, #0x04ff
    msr mair_el1, x1

    // 48-bit TTBR0/TTBR1, 4 KiB granules, inner-shareable WBWA, 40-bit PA.
    movz x1, #0x3510
    movk x1, #0xb510, lsl #16
    movk x1, #0x0002, lsl #32
    msr tcr_el1, x1
    msr ttbr0_el1, x20
    msr ttbr1_el1, x20
    dsb ish
    isb

    mrs x1, sctlr_el1
    orr x1, x1, #0x1000
    mov x2, #0x1d
    orr x1, x1, x2
    msr sctlr_el1, x1
    isb

    ldr x1, =.Lboot_higher_half
    br x1

.section .text, "ax"
.align 4
.Lboot_higher_half:
    ldr x1, =0xFFFF800000000000
    ldr x2, =__boot_stack_end
    orr x2, x2, x1
    mov sp, x2

    mov x0, x19
    bl aarch64_prepare_boot_protocol

    ldr x1, =0xFFFF800000000000
    orr x0, x0, x1
    bl kernel_entry

.Lhalt:
    msr daifset, #0xf
    wfe
    b .Lhalt
