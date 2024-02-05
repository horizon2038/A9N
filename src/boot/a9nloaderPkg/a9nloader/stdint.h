#ifndef STDINT_H
#define STDINT_H

#include <Uefi.h>

// signed
typedef short int16_t;
typedef int int32_t;
typedef long long int int64_t;

// unsigned
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
typedef UINTN uintmax_t;
#endif
