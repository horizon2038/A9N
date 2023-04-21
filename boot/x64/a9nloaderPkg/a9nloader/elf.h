#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef uint64_t elf64_addr;
typedef uint16_t elf64_half;
typedef uint64_t elf64_off;
typedef int32_t elf64_sword;
typedef int64_t elf64_sxword;
typedef uint32_t elf64_word;
typedef uint64_t elf64_lword;
typedef uint64_t elf64_xword;

typedef struct
{
    unsigned char	e_ident[16];
	elf64_half	e_type;
	elf64_half	e_machine;
	elf64_word	e_version;
	elf64_addr	e_entry;
	elf64_off	e_phoff;
	elf64_off	e_shoff;
	elf64_word	e_flags;
	elf64_half	e_ehsize;
	elf64_half	e_phentsize;
	elf64_half	e_phnum;
	elf64_half	e_shentsize;
	elf64_half	e_shnum;
	elf64_half	e_shstrndx;
} elf64_ehdr;

#define	ET_NONE 0
#define	ET_REL 1
#define	ET_EXEC 2
#define	ET_DYN 3
#define	ET_CORE 4

typedef struct
{
    elf64_word p_type;
    elf64_word p_flags;
    elf64_off p_offset;
    elf64_addr p_vaddr;
    elf64_addr p_paddr;
    elf64_xword p_filesz;
    elf64_xword p_memsz;
    elf64_xword p_align;
} elf64_phdr;

#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7

#endif ELF_H