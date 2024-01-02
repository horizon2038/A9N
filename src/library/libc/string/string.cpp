#include <library/libc/string.hpp>

namespace std
{
    void *memset(void *buffer, char value, size_t buffer_size)
    {
        uint8_t *buffer_pointer = reinterpret_cast<uint8_t *>(buffer);

        for (size_t i = 0; i < buffer_size; i++)
        {
            *buffer_pointer = value;
            buffer_pointer++;
        }

        return buffer;
    }

    void *memcpy(void *buffer, const void *source, size_t size)
    {
        uint8_t *buffer_pointer = reinterpret_cast<uint8_t *>(buffer);
        const uint8_t *source_pointer
            = reinterpret_cast<const uint8_t *>(source);

        for (size_t i = 0; i < size; i++)
        {
            *buffer_pointer = *source_pointer;
            buffer_pointer++;
            source_pointer++;
        }

        return buffer;
    }

    int memcmp(const void *source_1, const void *source_2, size_t size)
    {
        const uint8_t *pointer_1 = reinterpret_cast<const uint8_t *>(source_1);
        const uint8_t *pointer_2 = reinterpret_cast<const uint8_t *>(source_2);

        while (size-- > 0)
        {
            if (*pointer_1 != *pointer_2)
            {
                return (*pointer_1 - *pointer_2);
            }
            pointer_1++;
            pointer_2++;
        }
        return 0;
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
        return *reinterpret_cast<const unsigned char *>(source_1)
             - *reinterpret_cast<const unsigned char *>(source_2);
    }

    int strncmp(const char *source_1, const char *source_2, size_t size)
    {
        char result = 0;

        while (size)
        {
            if ((result = *source_1 - *source_2++) != 0 || !(*source_1++))
            {
                break;
            }
            size--;
        }

        return result;
    }

}
