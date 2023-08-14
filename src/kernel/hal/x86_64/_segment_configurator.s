global _load_gdt, _load_segment_register

gdtr DW 0
    DQ 0

_load_gdt:
    mov [gdtr], di
    mov [gdtr + 2], rsi
    lgdt [gdtr]
    ret

_load_segment_register:
    push rdi
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs
    ; if you setting up of code segment register: CS, you should use far return and set up value.
    mov ax, 0x10
    ; ax register value is 0x10, but safe to use null.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
