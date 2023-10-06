#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

namespace library
{
    void *memset(void *buffer, char value, size_t count);
    void *memcpy(void *buffer, const void *source, size_t size);
    char *strcpy(char *buffer, const char *source);
    int strcmp(const char *source_1, const char *source_2);
}

#endif
