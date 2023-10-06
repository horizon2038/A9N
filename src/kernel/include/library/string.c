#include "string.h"

namespace library 
{
    void *memset(void *buffer, char value, size_t buffer_size)
    {
        uint8_t *buffer_pointer = (uint8_t*)buffer;

        for (size_t i = 0; i < buffer_size; i++)
        {
            *buffer_pointer = value;
            buffer_pointer++;
        }

        return buffer;
    }

    void *memcpy(void *buffer, const void *source, size_t size)
    {
        uint8_t *buffer_pointer = (uint8_t*)buffer;
        const uint8_t *source_pointer = (const uint8_t*)source;
        
        for (size_t i = 0; i < size; i++)
        {
            *buffer_pointer = *source_pointer;
            buffer_pointer++;
            source_pointer++;
        }

        return buffer;
    }

    char *strcpy(char *buffer, const char *source)
    {
        char *buffer_pointer = buffer;
        
        while (*source != '\0')
        {
            *buffer_pointer = *source;
            buffer_pointer++;
            source++;
        }

        *buffer_pointer = '\0';
        
        return buffer;
    }

    int strcmp(const char *source_1, const char *source_2)
    {
        while (*source_1 && *source_2)
        {
            if (*source_1 != *source_2)
            {
                break;
            }
            
            source_1++;
            source_2++;
        }
        return *(unsigned char*)source_1 - *(unsigned char*)source_2;
    }
}
