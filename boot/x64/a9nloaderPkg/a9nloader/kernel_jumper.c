#include "kernel_jumper.h"

#include <Uefi.h>
#include <Library/UefiLib.h>

void jump_kernel(uint64_t entry_point_address)
{
    typedef int (__attribute__((sysv_abi)) entry_point) (void);
    entry_point *kernel_entry_point = (entry_point *)entry_point_address;
    uint64_t code = kernel_entry_point();
    Print(L"kernel code: %08llu\r\n", code);
}