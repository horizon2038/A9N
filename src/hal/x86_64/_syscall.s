section .text

global _write_msr
global _read_msr

_write_msr:
    mov rdx, rsi
    shr rdx, 32
    mov eax, esi
    mov ecx, edi
    wrmsr
    ret

_read_msr:
    rdmsr
    shl rdx, 32
    or rdx, rax
    mov rax, rdx
    ret
