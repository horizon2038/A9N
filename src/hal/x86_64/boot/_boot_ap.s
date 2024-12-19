; booting the Application Processor (AP)

; the BSP (Bootstrap Processor) is initially started in Long Mode by UEFI.
; on the other hand, the AP starts in Real Mode; so it needs to transition to Long Mode.

; to make them accessible in Real Mode, they are copied to the lower address.
section .boot

global __boot_ap_trampoline_original_start, __boot_ap_trampoline_original_end

extern __boot_ap_trampoline_start

bits 16
align 16
__boot_ap_trampoline_original_start:

boot_ap_16:
    cli
    cld

    ; init segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; linker defined
    extern __boot_ap_trampoline_gdtr
    lgdt [__boot_ap_trampoline_gdtr]

    ; enable protect mode
    mov eax, cr0
    or eax, 0x01 ; set PE
    mov cr0, eax

    db 0xea ; jmp ptr16:32
    dw boot_ap_32 - boot_ap_16 + __boot_ap_trampoline_start
    db 0x08, 0x00


bits 32
boot_ap_32:
    ; configure segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; enable some flags
    mov ecx, cr4
    or ecx, 1 << 5 ; enable PAE
    or ecx, 1 << 7 ; enable PGE
    mov cr4, ecx

    ; enable long mode
    mov ecx, 0xc0000080 ; EFER MSR
    rdmsr
    or eax, 1 << 8
    wrmsr  

    ; configure the initial page table of the kernel
    ; NOTE: since the table is already mapped at the time of BP initialization,
    ; it only needs to be set to CR3 to complete the process.
    extern __kernel_pml4
    lea eax, [__kernel_pml4]
    mov cr3, eax

    ; enable paging
    mov eax, cr0
    or eax, 1 << 31 ; set PG
    mov cr0, eax

    ; TODO: fix this (protect -> long)
    jmp 0x18:boot_ap_64

bits 64
boot_ap_64:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; read local apic id
    mov ecx, 0xfee00020
    mov eax, [rcx]
    shr eax, 24
    mov ebx, eax

    extern __boot_stack_start
    lea rdi, [__boot_stack_start]
    mov eax, ebx
    shl eax, 12
    add rdi, rax
    add rdi, 0x1000
    mov rsp, rdi

    jmp boot_ap_loop

boot_ap_loop:
    ; usually this code is not called.
    cli
    hlt
    jmp boot_ap_loop


align 16
global __boot_ap_gdt_start, __boot_ap_gdt_end
__boot_ap_gdt_start:
    dq 0x0000000000000000 ; null descriptor
    dq 0x00cf9a000000ffff ; 0x08 code segment 32bit
    dq 0x00cf92000000ffff ; 0x10 data segment 32bit
    dq 0x00af9a000000ffff ; 0x18 code segment 64bit

__boot_ap_gdt_end:

__boot_ap_trampoline_original_end:


