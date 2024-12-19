BITS 64
section .text

global _cpuid

; void _cpuid(uint32_t leaf, uint32_t subleaf, cpuid_info *info);
; leaf : rdi
; subleaf : rsi
; info : rdx

_cpuid:
    ; prologue
    push rbp
    mov rbp, rsp

    ; rbx is callee-saved
    push rbx
    
    ; 1st argument : cpuid type
    mov rax, rdi
    mov rcx, rsi

    ; save rdx
    mov rdi, rdx

    ; run
    cpuid

    ; store results
    mov [rdi + 0x00], eax
    mov [rdi + 0x04], ebx
    mov [rdi + 0x08], ecx
    mov [rdi + 0x0c], edx

    ; restore callee-saved registers
    pop rbx

    ; epilogue
    mov rsp, rbp
    pop rbp
    ret
