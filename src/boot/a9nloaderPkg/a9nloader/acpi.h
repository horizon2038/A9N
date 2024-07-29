#ifndef ACPI_H
#define ACPI_H

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Uefi.h>

void *find_rsdp(EFI_SYSTEM_TABLE *system_table);

#endif
