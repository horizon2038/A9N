#include "kernel_loader.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include "file_reader.h"
#include "elf_info_logger.h"

static EFI_STATUS read_elf_header(EFI_FILE_PROTOCOL *, elf64_header **);
static EFI_STATUS
    read_elf_program_header(EFI_FILE_PROTOCOL *, elf64_header *, elf64_program_header **);
static EFI_STATUS
    load_elf_segment(EFI_FILE_PROTOCOL *, elf64_header *, elf64_program_header *);
static EFI_STATUS
    allocate_kernel_memory(elf64_header *, elf64_program_header *);
static void
    locate_elf_segment(elf64_header *, elf64_program_header *, uint64_t);
static void zero_clear(elf64_program_header *);

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL *kernel, uint64_t *entry_point)
{
    EFI_STATUS            efi_status = EFI_SUCCESS;
    elf64_header         *loaded_elf64_header;
    elf64_program_header *loaded_elf64_program_header_table;

    efi_status = read_elf_header(kernel, &loaded_elf64_header);
    if (EFI_ERROR(efi_status))
        return efi_status;
    efi_status = read_elf_program_header(
        kernel,
        loaded_elf64_header,
        &loaded_elf64_program_header_table
    );
    if (EFI_ERROR(efi_status))
        return efi_status;
    efi_status = load_elf_segment(
        kernel,
        loaded_elf64_header,
        loaded_elf64_program_header_table
    );
    if (EFI_ERROR(efi_status))
        return efi_status;

    *entry_point = loaded_elf64_header->entry_point_address;
    // *entry_point = loaded_elf64_header->entry_point_address -
    // 0xffff800000000000;

    return efi_status;
}

static EFI_STATUS
    read_elf_header(EFI_FILE_PROTOCOL *kernel, elf64_header **header)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    efi_status = read_file(kernel, 0, sizeof(elf64_header), (void **)header);
    print_elf_header_info(*header);

    return efi_status;
}

static EFI_STATUS read_elf_program_header(
    EFI_FILE_PROTOCOL     *kernel,
    elf64_header          *header,
    elf64_program_header **program_header
)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    uint64_t program_header_table_size
        = sizeof(elf64_program_header) * header->program_header_number;
    // Print(L"");
    efi_status = read_file(
        kernel,
        header->program_header_offset,
        program_header_table_size,
        (void **)program_header
    );

    return efi_status;
}

static EFI_STATUS load_elf_segment(
    EFI_FILE_PROTOCOL    *kernel,
    elf64_header         *header,
    elf64_program_header *program_header_table
)
{
    EFI_STATUS            efi_status = EFI_SUCCESS;
    elf64_program_header *program_header;
    void                 *buffer;

    efi_status = allocate_kernel_memory(header, program_header_table);
    if (EFI_ERROR(efi_status))
        return efi_status;
    for (uint16_t i = 0; i < header->program_header_number; ++i)
    {
        program_header = &(program_header_table[i]);
        if (program_header->type != PT_LOAD)
        {
            continue;
        }
        print_elf_program_header_info(program_header);
        efi_status = read_file(
            kernel,
            program_header->offset,
            program_header->file_size,
            &buffer
        );
        if (EFI_ERROR(efi_status))
            return efi_status;
        locate_elf_segment(header, program_header, (uint64_t)buffer);
        zero_clear(program_header);
    }

    return efi_status;
}

static EFI_STATUS allocate_kernel_memory(
    elf64_header         *header,
    elf64_program_header *program_header_table
)
{
    EFI_STATUS            efi_status = EFI_SUCCESS;
    elf64_program_header *program_header;
    uint64_t              start_segment_address;
    uint64_t              end_segment_address;
    uint64_t              segment_size;

    start_segment_address = MAX_UINT64;
    end_segment_address   = 0;

    for (uint16_t i = 0; i < header->program_header_number; ++i)
    {
        program_header = &(program_header_table[i]);
        if (program_header->type != PT_LOAD)
        {
            continue;
        }
        start_segment_address
            = MIN(start_segment_address, program_header->physical_address);
        end_segment_address = MAX(
            end_segment_address,
            program_header->physical_address + program_header->memory_size
        );
    }
    segment_size
        = EFI_SIZE_TO_PAGES(end_segment_address - start_segment_address);
    efi_status = gBS->AllocatePages(
        AllocateAddress,
        EfiLoaderData,
        segment_size,
        &start_segment_address
    );

    return efi_status;
}

static void locate_elf_segment(
    elf64_header         *header,
    elf64_program_header *program_header,
    uint64_t              buffer
)
{
    CopyMem(
        (void *)(program_header->physical_address),
        (void *)buffer,
        program_header->file_size
    );
}

static void zero_clear(elf64_program_header *program_header)
{
    if (program_header->file_size < program_header->memory_size)
    {
        gBS->SetMem(
            (void *)(program_header->physical_address
                     + program_header->file_size),
            program_header->memory_size - program_header->file_size,
            0
        );
    }
}
