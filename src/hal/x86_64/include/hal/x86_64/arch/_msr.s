section .text

global _write_msr
global _read_msr

_write_msr:
    ; rdi = msr address
    ; rsi = value
    mov ecx, edi
    mov eax, esi
    mov rdx, rsi
    shr rdx, 32
    wrmsr
    ret

_read_msr:
    ; rdi = msr address
    mov ecx, edi
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret

