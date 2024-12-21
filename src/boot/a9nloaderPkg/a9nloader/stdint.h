#ifndef STDINT_H
#define STDINT_H

#include <Uefi.h>

// signed
typedef INT16 int16_t;
typedef INT32 int32_t;
typedef INT64 int64_t;
typedef INTN  intmax_t;

// unsigned
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;
typedef UINTN  uintmax_t;

#endif
