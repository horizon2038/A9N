#ifndef ELF_H
#define ELF_H

#include "stdint.h"

typedef uint64_t elf64_address;
typedef uint16_t elf64_half;
typedef uint64_t elf64_offset;
typedef int32_t  elf64_sword;
typedef int64_t  elf64_sxword;
typedef uint32_t elf64_word;
typedef uint64_t elf64_lword;
typedef uint64_t elf64_xword;

typedef struct
{
    unsigned char identifier[16];
    elf64_half    type;
    elf64_half    machine;
    elf64_word    version;
    elf64_address entry_point_address;
    elf64_offset  program_header_offset;
    elf64_offset  section_header_offset;
    elf64_word    flags;
    elf64_half    size;
    elf64_half    program_header_size;
    elf64_half    program_header_number;
    elf64_half    section_header_size;
    elf64_half    section_header_number;
    elf64_half    section_header_string_table_index;
} elf64_header;

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

typedef struct
{
    elf64_word    type;
    elf64_word    flags;
    elf64_offset  offset;
    elf64_address virtual_address;
    elf64_address physical_address;
    elf64_xword   file_size;
    elf64_xword   memory_size;
    elf64_xword   alignment;
} elf64_program_header;

#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7

typedef struct
{
    elf64_word    name;
    elf64_word    type;
    elf64_xword   flags;
    elf64_address virtual_address;
    elf64_offset  offset;
    elf64_xword   size;
    elf64_word    link;
    elf64_word    info;
    elf64_xword   alignment;
    elf64_xword   entry_size;
} elf64_section_header;

typedef struct
{
    elf64_word    name;
    unsigned char info;
    unsigned char other;
    elf64_half    section_header_index;
    elf64_address value;
    elf64_xword   size;
} elf64_symbol;

#endif
