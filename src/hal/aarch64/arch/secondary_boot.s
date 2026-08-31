.section .boot.secondary, "ax"
.align 7

.global aarch64_secondary_entry
.type aarch64_secondary_entry, %function

.extern __boot_stack_start
.extern __kernel_l0
.extern aarch64_ap_entry

aarch64_secondary_entry:
    msr daifset, #0xf

    // The supported QEMU virt and BCM2711 topologies use Aff0 as their
    // physical core number. Select a private temporary boot stack before C++.
    mrs x19, mpidr_el1
    and x19, x19, #0xff
    ldr x20, =__boot_stack_start
    add x20, x20, #0x2000
    add x20, x20, x19, lsl #13
    mov sp, x20

    mrs x1, CurrentEL
    cmp x1, #0x8
    b.eq .Lsecondary_drop_from_el2
    cmp x1, #0x4
    b.eq .Lsecondary_el1

.Lsecondary_unsupported_el:
    wfe
    b .Lsecondary_unsupported_el

.Lsecondary_drop_from_el2:
    msr sp_el1, x20
    mov x1, #(1 << 31)
    msr hcr_el2, x1
    msr cptr_el2, xzr
    mov x1, #3
    msr cnthctl_el2, x1
    msr cntvoff_el2, xzr
    adr x1, .Lsecondary_el1
    msr elr_el2, x1
    mov x1, #0x3c5
    msr spsr_el2, x1
    eret

.Lsecondary_el1:
    mov sp, x20

    // Reuse the page tables prepared by the boot core.
    mov x1, #0x04ff
    msr mair_el1, x1
    movz x1, #0x3510
    movk x1, #0xb510, lsl #16
    movk x1, #0x0002, lsl #32
    msr tcr_el1, x1
    ldr x20, =__kernel_l0
    msr ttbr0_el1, x20
    msr ttbr1_el1, x20
    dsb sy
    isb

    mrs x1, sctlr_el1
    orr x1, x1, #0x1000
    mov x2, #0x1d
    orr x1, x1, x2
    msr sctlr_el1, x1
    isb

    ldr x1, =.Lsecondary_higher_half
    br x1

.section .text, "ax"
.align 4
.global aarch64_secondary_entry_address
.type aarch64_secondary_entry_address, %function
aarch64_secondary_entry_address:
    ldr x0, 1f
    ret
    .align 3
1:
    .quad aarch64_secondary_entry

.align 4
.Lsecondary_higher_half:
    ldr x2, =0xFFFF800000000000
    mov x1, sp
    orr x1, x1, x2
    mov sp, x1
    bl aarch64_ap_entry

.Lsecondary_halt:
    msr daifset, #0xf
    wfe
    b .Lsecondary_halt
