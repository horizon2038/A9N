#include <Uefi.h>
#include <Library/UefiLib.h>

EFI_STATUS
EFIAPI
efi_main (IN EFI_HANDLE image_handle, IN EFI_SYSTEM_TABLE *system_table)
{
    system_table->ConOut->ClearScreen(system_table->ConOut);
    system_table->ConOut->OutputString(system_table->ConOut, L"A9N Loader\n");
    while(1);
    return 0;
}