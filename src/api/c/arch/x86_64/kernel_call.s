section .text

global _a9n_kernel_call

_a9n_kernel_call
    ; in system v abi, the argument of index 3 is passed as RCX,
    ; but RCX is reserved by the `syscall` instruction.
    ; therefore, the index is shifted by one.
    mov r9, r8
    mov r8, rcx
    o64 syscall

    ; result is stored to ipc_buffer; we should parse ipc_buffer contents.
    ret
