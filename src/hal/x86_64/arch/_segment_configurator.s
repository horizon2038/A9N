default rel
section .text

global _load_gdt
global _load_segment_register

global _load_task_register

global _write_gs_base
global _write_fs_base
global _read_gs_base
global _read_fs_base

gdtr DW 0
    DQ 0

_load_gdt:
    mov [gdtr], di; setup gdt
    mov [gdtr + 2], rsi
    lgdt [gdtr]
    ret

_load_segment_register:
    push rdi
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs:
    mov ax, di
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    xor rax, rax
    mov ss, rax
    ret

_load_task_register:
    ltr di
    ret

; void _write_gs_base(virtual_address gs_base_address)
_write_gs_base:
    wrgsbase rdi
    ret

; void _write_fs_base(virtual_address fs_base_address)
_write_fs_base:
    wrfsbase rdi
    ret

; virtual_address _read_gs_base(void)
_read_gs_base:
    rdgsbase rax
    ret

; virtual_address _read_fs_base(void)
_read_fs_base:
    rdfsbase rax
    ret
