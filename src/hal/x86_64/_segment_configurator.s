section .text

global _load_gdt
global _load_segment_register

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

.reload_cs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
