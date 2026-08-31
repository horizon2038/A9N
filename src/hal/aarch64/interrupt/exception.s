.section .text, "ax"
.align 11

.global aarch64_exception_vectors
.type aarch64_exception_vectors, %function
aarch64_exception_vectors:
    // Current EL with SP0.
    b aarch64_sync_current
    .balign 128
    b aarch64_irq_current
    .balign 128
    b aarch64_fatal_current
    .balign 128
    b aarch64_fatal_current
    .balign 128

    // Current EL with SPx.
    b aarch64_sync_current
    .balign 128
    b aarch64_irq_current
    .balign 128
    b aarch64_fatal_current
    .balign 128
    b aarch64_fatal_current
    .balign 128

    // Lower EL using AArch64.
    b aarch64_sync_lower
    .balign 128
    b aarch64_irq_lower
    .balign 128
    b aarch64_fatal_lower
    .balign 128
    b aarch64_fatal_lower
    .balign 128

    // Lower EL using AArch32 (unsupported).
    b aarch64_fatal_lower
    .balign 128
    b aarch64_fatal_lower
    .balign 128
    b aarch64_fatal_lower
    .balign 128
    b aarch64_fatal_lower
    .balign 128

.macro SAVE_FRAME kind
    sub sp, sp, #304
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
    mrs x2, sp_el0
    str x2, [sp, #248]
    mrs x2, elr_el1
    str x2, [sp, #256]
    mrs x2, spsr_el1
    str x2, [sp, #264]
    mrs x2, tpidr_el0
    str x2, [sp, #272]
    mrs x2, esr_el1
    str x2, [sp, #280]
    mrs x2, far_el1
    str x2, [sp, #288]
    mov x0, sp
    mov x1, #\kind
    bl aarch64_handle_exception
    b aarch64_restore_exception_context
.endm

aarch64_sync_current:
    SAVE_FRAME 0
aarch64_irq_current:
    SAVE_FRAME 1
aarch64_sync_lower:
    SAVE_FRAME 2
aarch64_irq_lower:
    SAVE_FRAME 3
aarch64_fatal_current:
    SAVE_FRAME 4
aarch64_fatal_lower:
    SAVE_FRAME 5

.type aarch64_restore_exception_context, %function
aarch64_restore_exception_context:
    // The returned context may belong to a different process, but SP still
    // points at this core's exception frame.
    add sp, sp, #304
    b aarch64_restore_context

.global aarch64_restore_context
.type aarch64_restore_context, %function
aarch64_restore_context:
    mov x30, x0
    ldr x1, [x30, #248]
    msr sp_el0, x1
    ldr x1, [x30, #256]
    msr elr_el1, x1
    ldr x1, [x30, #264]
    msr spsr_el1, x1
    ldr x1, [x30, #272]
    msr tpidr_el0, x1
    ldp x0, x1, [x30, #0]
    ldp x2, x3, [x30, #16]
    ldp x4, x5, [x30, #32]
    ldp x6, x7, [x30, #48]
    ldp x8, x9, [x30, #64]
    ldp x10, x11, [x30, #80]
    ldp x12, x13, [x30, #96]
    ldp x14, x15, [x30, #112]
    ldp x16, x17, [x30, #128]
    ldp x18, x19, [x30, #144]
    ldp x20, x21, [x30, #160]
    ldp x22, x23, [x30, #176]
    ldp x24, x25, [x30, #192]
    ldp x26, x27, [x30, #208]
    ldp x28, x29, [x30, #224]
    ldr x30, [x30, #240]
    eret

.global aarch64_save_floating_context
.type aarch64_save_floating_context, %function
aarch64_save_floating_context:
    stp q0, q1, [x0, #0]
    stp q2, q3, [x0, #32]
    stp q4, q5, [x0, #64]
    stp q6, q7, [x0, #96]
    stp q8, q9, [x0, #128]
    stp q10, q11, [x0, #160]
    stp q12, q13, [x0, #192]
    stp q14, q15, [x0, #224]
    stp q16, q17, [x0, #256]
    stp q18, q19, [x0, #288]
    stp q20, q21, [x0, #320]
    stp q22, q23, [x0, #352]
    stp q24, q25, [x0, #384]
    stp q26, q27, [x0, #416]
    stp q28, q29, [x0, #448]
    stp q30, q31, [x0, #480]
    mrs x1, fpcr
    str x1, [x0, #512]
    mrs x1, fpsr
    str x1, [x0, #520]
    ret

.global aarch64_restore_floating_context
.type aarch64_restore_floating_context, %function
aarch64_restore_floating_context:
    ldp q0, q1, [x0, #0]
    ldp q2, q3, [x0, #32]
    ldp q4, q5, [x0, #64]
    ldp q6, q7, [x0, #96]
    ldp q8, q9, [x0, #128]
    ldp q10, q11, [x0, #160]
    ldp q12, q13, [x0, #192]
    ldp q14, q15, [x0, #224]
    ldp q16, q17, [x0, #256]
    ldp q18, q19, [x0, #288]
    ldp q20, q21, [x0, #320]
    ldp q22, q23, [x0, #352]
    ldp q24, q25, [x0, #384]
    ldp q26, q27, [x0, #416]
    ldp q28, q29, [x0, #448]
    ldp q30, q31, [x0, #480]
    ldr x1, [x0, #512]
    msr fpcr, x1
    ldr x1, [x0, #520]
    msr fpsr, x1
    ret
