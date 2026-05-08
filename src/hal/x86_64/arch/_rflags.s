section .text

global _read_rflags

_read_rflags:
    pushfq
    pop rax

    ret
