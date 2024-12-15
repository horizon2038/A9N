# A9N Microkernel Bootstrap

## x86_64

### Boot Process

1. *A9NLoader* loads `kernel.elf` and `init.elf` into memory and transfers control to the kernel.
2. The kernel receives the memory map from *A9NLoader*, including the location where `init` is loaded, the virtual address of `init`, the size of `init`, and the *IPC Buffer* for `init`. Using this information, the kernel creates an *Init Server (Alpha)* and starts it.

> [!NOTE]
> The kernel does not recognize the `ELF` format, allowing the bootloader to use other formats besides `ELF`.

### Init Info 

*A9NLoader* provides `init` with a list of *Generic Capabilities* and the address of the *IPC Buffer* via `init_info`.
The location of `init_info` is specified in the linker script, and it resides between `__init_info_start` and `__init_info_end`, a reserved 4KiB area.

### IPC Buffer

