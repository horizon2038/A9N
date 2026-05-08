section .text

global _x64syscall

_x64syscall:
    mov rdi, 0xdeadbeaf
    mov rsi, 64
    mov rdx, 0xdeadbeaf
    mov r8, 0xdeadbeaf
    o64 syscall
    ret
