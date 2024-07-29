#include "kernel_jumper.h"

#include <Library/UefiLib.h>
#include <Uefi.h>

void jump_kernel(uint64_t entry_point_address, boot_info *target_boot_info)
{
    typedef int(__attribute__((sysv_abi)) entry_point)(boot_info *);
    entry_point *kernel_entry_point = (entry_point *)entry_point_address;
    uint64_t     code               = kernel_entry_point(target_boot_info);
    Print(L"kernel code: 0x%08llx\r\ndec: %08llu\r\n", code, code);
}
