section .text

global _syscall_handler

extern do_syscall

_syscall_handler:
.handle_syscall:
    ; +----------+--------------------+
    ; | Register |    Description     |
    ; +----------+--------------------+
    ; | RCX      | user RIP stored    |
    ; | R11      | user RFLAGS stored |
    ; +----------+--------------------+
    
    ; +-----------------------+
    ; | caller-saved(usable)  |
    ; +---------*-------------+
    ; | RAX     |             |
    ; | RCX     |    user RIP |
    ; | RDX     |             |
    ; | RSI     |             |
    ; | RDI     |             |
    ; | R8      |             |
    ; | R9      |             |
    ; | R10     |             |
    ; | R11     | user RFLAGS |
    ; | XMM ... |             |
    ; | YMM ... |             |
    ; | ZMM ... |             |
    ; +---------*-------------+

    swapgs

    ; temporarily save rsp
    mov rax, rsp

    ; load hardware_context
    mov rsp, [gs:0x08]

    ; need to reserve register space.
    add rsp, 0x08 * 19

    ; save rsp
    push rax
    ; save rflags
    push r11
    ; skip cs
    sub rsp, 0x08
    ; save rip
    push rcx

    ; these are saved to hardware_context
    push r15
    push r14
    push r13
    push r12
    ; skip r11 (RFLAGS)
    sub rsp, 0x08
    push r10
    push r9
    push r8
    push rbp
    push rsi
    push rdi
    push rdx
    ; skip rcx (RIP)
    sub rsp, 0x08
    push rbx
    ; skip rax
    ; since rax is top-level, no rsp operation is required

    ; setup do_syscall arguments
    ; the argument has already been set
    ; RDI : descriptor
    ; RSI : depth
    ; RDX : message_tag
    ; R8  : message_length
    mov rcx, r8

    ; use kernel_stack since do_syscall
    mov rsp, [gs:0x00]

    and rsp, qword -0x10
    sub rsp, 8 ; for call (return address)

    ; signature : void do_syscall(uint64_t descriptor, uint64_t depth, uint64_t message_tag, uint64_t message_length)
    call do_syscall

    add rsp, 0x08

.restore_context:
    ; load hardware_context
    mov rsp, [gs:0x08]

    ; these are restored from hardware_context

    ; skip rax
    ; rax is the return value of do_syscall.
    add rsp, 0x08
    pop rbx
    ; skip rcx (RIP)
    add rsp, 0x08
    pop rdx
    pop rdi
    pop rsi
    pop rbp
    pop r8
    pop r9
    pop r10
    ; skip r11 (RFLAGS)
    add rsp, 0x08
    pop r12
    pop r13
    pop r14
    pop r15

    ; restore syscall context
    ; rcx = rip
    pop rcx
    ; skip CS
    add rsp, 0x08
    ; r11 = rflags
    pop r11
    pop rsp

    swapgs

    o64 sysret
