; section .boot progbits alloc exec nowrite align=16
section .boot alloc exec 
bits 64

extern __kernel_pml4
extern __kernel_pdpt
extern __kernel_pd

extern __kernel_start
extern __kernel_end

bits 64
section .boot
global _boot_kernel_gateway
_boot_kernel_gateway:
    ; initialize
    cli
    cld

    ; setup boot_stack
    extern __boot_stack_end
    lea rsp, [__boot_stack_end]
    push rdi ; push boot_info_address

    lgdt [_boot_gdtr]
    call _load_segment_register
    
    lea rax, [_boot_lower_half]
    jmp rax

; temporary GDT;

bits 64
section .boot align=16
_boot_gdtr:
    dw gdt_end - gdt - 16
    dq gdt

bits 64
section .boot align=16
gdt:
    dq 0x0000000000000000
    dq 0x00af9a000000ffff
    dq 0x00cf92000000ffff
    dq 0x00cf9a000000ffff
gdt_end:


bits 64
section .boot

_load_segment_register:
    push 0x08
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs
    mov ax, 0x0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

_boot_lower_half:
    ; setup paging
    extern init_page_table
    call init_page_table
    lea rax, [__kernel_pml4] 
    mov cr3, rax

    ; convert stack_address to virtual_address.
    mov rax, 0xFFFF800000000000
    or rsp, rax

    mov rax, qword _boot_higher_half
    jmp rax

bits 64
section .text
_boot_higher_half:
    ; setup entry point
    extern kernel_entry

    ; boot_info_configuration
    mov rax, 0xFFFF800000000000
    pop rdi
    or rdi, rax

    mov rax, kernel_entry
    call rax

    ; jump kernel_entry

none:
    ; usually this code is not called.
    cli
    hlt
    jmp none

