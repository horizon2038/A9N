#ifndef ACPI_H
#define ACPI_H

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

void *find_rsdp(EFI_SYSTEM_TABLE *system_table);

#endif
