; booting the Application Processor (AP)

; the BSP (Bootstrap Processor) is initially started in Long Mode by UEFI.
; on the other hand, the AP starts in Real Mode; so it needs to transition to Long Mode.

; to make them accessible in Real Mode, they are copied to the lower address.
global __boot_ap_trampoline_original_start, __boot_ap_trampoline_original_end

bits 16
align 16
__boot_ap_trampoline_original_start:
.boot_ap_16:
    cli
    cld

    ; init segment registers
    mov cs, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; load temporary gdt
    ; extern _boot_gdtr
    ; lgdt [_boot_gdtr]

    ; enable protect mode
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; reload cs; with far jump
    ; NASM : jmp far []

bits 32
.boot_ap_32:
    ; configure segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; load temprary boot stack


__boot_ap_trampoline_original_end:

; transition from Protect Mode to Long Mode
global _enable_long_mode

bits 32
_enable_long_mode:
    ; enable some flags
    mov ecx, cr4
    or ecx, 1 << 5 ; enable PAE
    or ecx, 1 << 7 ; enable PGE
    mov cr4, eax

    ; configure the initial page table of the kernel
    ; NOTE: since the table is already mapped at the time of BP initialization,
    ; it only needs to be set to CR3 to complete the process.
    extern __kernel_pml4
    lea eax, [__kernel_pml4]
    mov cr3, eax

    ; switch to long mode!
    mov ecx, 0xc0000080 ; EFER MSR
    rdmsr
    or eax, 1 << 8
    wrmsr  

    ; enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax


