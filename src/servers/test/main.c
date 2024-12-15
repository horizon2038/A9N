void main();

void _start()
{
    // some initialization
    // 1. configure stack
    // 2. configure init_info
    main();
}

// init info definition
extern char __init_info_start[];
extern char __init_info_end[];

// init stack definition
extern char __init_stack_start[];
extern char __init_stack_end[];

// init ipc buffer definition
extern char __init_ipc_buffer_start[];
extern char __init_ipc_buffer_end[];

void main()
{
    while (0)
    {
        char result;
        asm volatile(
            "mov $0xdeadbeaf, %%rdi\n"  // Move 0xdeadbeaf into RDI
            "mov $64, %%rsi\n"          // Move 64 into RSI
            "mov $0xdeadbeaf, %%rdx\n"  // Move 0xdeadbeaf into RDX
            "mov $0xdeadbeaf, %%r8\n"   // Move 0xdeadbeaf into R8
            "syscall\n"                 // Perform the syscall
            : "=a"(result)              // Output the result in RAX (syscall return value)
            :                           // No inputs
            : "rdi", "rsi", "rdx", "r8" // Clobbered registers
        );
        asm volatile("nop");
    }
}
