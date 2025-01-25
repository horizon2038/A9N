global _interrupt_handlers

global _restore_kernel_context
global _restore_user_context

extern do_irq_from_kernel
extern do_irq_from_user

align 16
_interrupt_handlers:

%assign i 0
%rep 256
interrupt_handler_%+i:
    align 16

    ; stop interrupt
    cli

    ; Exception { 8, 10, 11, 12, 13, 14, 17, 21, 29, 30 }: with error code.
    %if i == 8 || (10<= i && i <= 14) || i == 17 || i == 21 || i = 29 || i = 30
        ; error_code has been pushed.
    %else
        ; since the error code does not exist; a dummy value is pushed.
        push qword 0
    %endif

    push qword i
    jmp interrupt_handler_common

    align 16
%assign i i+1
%endrep

interrupt_handler_common:
    ;  current kernel stack :
    ;  +-------+------------+---------+
    ;  | +0x30 | SS         |         |
    ;  | +0x28 | RSP        |         |
    ;  | +0x20 | RFLAGS     |         |
    ;  | +0x18 | CS         | for CPL |
    ;  | +0x10 | RIP        |         |
    ;  | +0x08 | error code |         |
    ;  | +0x00 | irq number | current |
    ;  +-------+------------+---------+
    ; `swapgs` must be used only for user -> kernel and kernel -> user.
    ; in other words, `swapgs` are not allowed for kernel -> kernel transitions.

    ; the lower 2 bits of CS register represent the CPL (current privilege level).
    ; to check the source of the transition,
    ; take the CS register out of the IRET Frame and check if it's 3 (0b11).
    test qword [rsp + 0x18], 3
    jnz .from_user 

.from_kernel:
    ; cmp qword [rsp + 0x28], 0 
    ; jnz .from_user

    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rsi
    push rdi
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, [rsp + 0x08 * 15] ; irq_number
    mov rsi, [rsp + 0x08 * 16] ; error_code

    ; align 16-byte boundary 
    and rsp, qword -0x10
    sub rsp, 8 ; for call (return address)

    ; void do_irq_from_kernel(irq_number, error_code)
    call do_irq_from_kernel

    add rsp, 0x10

    ; unreachable

.from_user:
    ; `swapgs` absolutely must be executed only when transitioning from user!
    swapgs

    ; at this point, the kernel stack from TSS.RSP0 is loaded into RSP.
    ; NOTE: error code and IRET Frame are loaded into kernel stack; not hardware_context.
    ; what we need to do is to load hardware_context into RSP and store registers into it.
    mov rsp, [gs:0x08]

    ; need to reserve register space.
    add rsp, 0x08 * 15 

    ; these are saved to hardware_context
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rsi
    push rdi
    push rdx
    push rcx
    push rbx
    push rax

    ; in order to move IRET Frame from kernel_stack to hardware_context,
    ; load the address to kernel_stack to RAX insted of RSP.
    mov rax, [gs:0x00]

    ; configure stack offset : sizeof(registers) + sizeof(iret_frame) + sizeof(segment_base)
    add rsp, 0x08 * 22 

    ; switch to user segment base
    swapgs

    ; save user segment base
    rdfsbase rbx
    push rbx
    rdgsbase rbx 
    push rbx

    ; switch to kernel segment base
    swapgs

    ; save IRET Frame 
    ; `push` decrements the RSP and then stores the value !
    ; therefore, SS becomes [rax - 0x08] instead of [rax - 0x00] !
    push qword [rax - 0x08] ; SS
    push qword [rax - 0x10] ; RSP
    push qword [rax - 0x18] ; RFLAGS
    push qword [rax - 0x20] ; CS
    push qword [rax - 0x28] ; RIP

    mov rdi, [rax - 0x38] ; irq_number 
    and rdi, 0xFF
    mov rsi, [rax - 0x30] ; error_code

    ; use kernel_stack since do_irq
    mov rsp, [gs:0x00]

    ; align 16-byte boundary
    and rsp, qword -0x10

    ; void do_irq_from_user(irq_number, error_code)
    call do_irq_from_user

    add rsp, 0x08

    ; unreachable

_restore_kernel_context:
    mov rsp, [gs:0x00]
    ; skip SS, RSP, RFLAGS, CS, RIP, error_code, irq_number
    add rsp, 0x08 * 8
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rdi
    pop rsi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    ; skip error_code + irq_number
    add rsp, 0x08 * 2

    o64 iret

_restore_user_context:
    ; load hardware_context
    mov rsp, [gs:0x08]

    swapgs
    ; restore segment base
    mov rbx, [rsp + 0x08 * 20]
    wrgsbase rbx
    mov rbx, [rsp + 0x08 * 21]
    wrfsbase rbx
    swapgs

    ; these are restored from hardware_context
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rdi
    pop rsi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    mov rsp, [gs:0x08]
    add rsp, 0x08 * 15


    ; mov rax, [gs:0x08]

    ; restore IRET Frame
    ; push qword [rax + 0x08 * 19] ; SS
    ; push qword [rax + 0x08 * 18] ; RSP
    ; push qword [rax + 0x08 * 17] ; RFLAGS
    ; push qword [rax + 0x08 * 16] ; CS
    ; push qword [rax + 0x08 * 15] ; RIP

    ; since we are going back to user from kernel, it is necessary to re-swap the swapped GS back.  
    swapgs

    o64 iret

