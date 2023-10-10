#include "print.hpp"

#include <interface/serial.hpp>
#include <stdint.h>
#include <stddef.h>

namespace kernel::utility
{
    print::print(hal::interface::serial &injected_serial) : _serial(injected_serial)
    {
    }

    void print::printf(const char *format, ...)
    {
        __builtin_va_list args;
        __builtin_va_start(args, format);
        vsprintf(print_buffer, format, args);
        __builtin_va_end(args);
        _serial.write_string_serial(print_buffer);
    }

    void print::vprintf(const char *format, __builtin_va_list args)
    {
        vsprintf(print_buffer, format, args);
        printf("%s", print_buffer);
    }

    void print::sprintf(char *buffer, const char *format, ...)
    {
        __builtin_va_list args;
        __builtin_va_start(args, format);
        vsprintf(buffer, format, args);
        __builtin_va_end(args);
    }

    void print::vsprintf(char *buffer, const char *format, __builtin_va_list args)
    {
        char* destination = buffer;

        for (const char *pointer = format; *pointer != '\0'; ++pointer)
        {
            if (*pointer == '%')
            {
                ++pointer;
                process_format(&destination, &pointer, args);
            }
            else
            {
                write_char(&destination, *pointer);
            }
        }
        write_char(&destination, '\0');
    }

    void print::process_format(char** destination, const char** format_pointer, __builtin_va_list args)
    {
        int width = 0;
        bool zero_pad = false;

        while (true)
        {
            if (**format_pointer == '0' && !width)
            {
                zero_pad = true;
                ++(*format_pointer);
            }
            else if (**format_pointer >= '1' && **format_pointer <= '9')
            {
                width = width * 10 + (**format_pointer - '0');
                ++(*format_pointer);
            }
            else if (**format_pointer == '0' && width)
            {
                width = width * 10;
                ++(*format_pointer);
            }
            else
            {
                break;
            }
        }

        switch (**format_pointer)
        {
            case 'd':
                write_int(destination, __builtin_va_arg(args, int), width, zero_pad);
                break;

            case 'x':
                write_hex(destination, __builtin_va_arg(args, unsigned int), width, zero_pad, false);
                break;

            case 'X':
                write_hex(destination, __builtin_va_arg(args, unsigned int), width, zero_pad, true);
                break;

            case 's':
                write_string(destination, __builtin_va_arg(args, char*), width);
                break;

            case 'p':
                write_pointer(destination, __builtin_va_arg(args, void*));
                break;

            default:
                write_char(destination, '%');
                write_char(destination, **format_pointer);
                break;
        }
    }

    void print::write_char(char** destination, char target_char)
    {
        if (*destination)
        {
            **destination = target_char;
            (*destination)++;
        }
    }

    void print::write_string(char** destination, char *target_string, int width)
    {
        int length = 0;

        for (char* pointer = target_string; *pointer != '\0'; ++pointer)
        {
            ++length;
        }

        int padding = width - length;

        while (padding-- > 0)
        {
            write_char(destination, ' ');
        }

        while(*target_string != '\0')
        {
            write_char(destination, *target_string);
            target_string++;
        }
    }

    void print::write_int(char** destination, int count, int width, bool zero_pad)
    {
        if (count < 0)
        {
            write_char(destination, '-');
            count = -count;
            --width;
        }

        char buffer[10];
        char* ptr = buffer + 10;

        do {
            *--ptr = '0' + (count % 10);
            count /= 10;
            --width;
        } while (count > 0);

        while (width-- > 0)
        {
            write_char(destination, zero_pad ? '0' : ' ');
        }

        while (ptr < buffer + 10)
        {
            write_char(destination, *ptr++);
        }
    }

    void print::write_hex(char** destination, unsigned int count, int width, bool zero_pad, bool uppercase)
    {
        char buffer[8];
        char* pointer = buffer + 8;

        do {
            int digit = count % 16;
            *--pointer = (digit < 10) ? ('0' + digit) : ((uppercase ? 'A' : 'a') + digit - 10);
            count /= 16;
            --width;
        } while (count > 0);

        char pad_char = zero_pad ? '0' : ' ';

        while (width-- > 0)
        {
            write_char(destination, pad_char);
        }

        while (pointer < buffer + 8)
        {
            write_char(destination, *pointer++);
        }
    }

    void print::write_pointer(char** destination, const void* pointer)
    {
        static const char* hex_digits = "0123456789abcdef";
        unsigned long value = reinterpret_cast<unsigned long>(pointer);

        write_char(destination, '0');
        write_char(destination, 'x');

        for (int i = (sizeof(pointer) * 8) - 4; i >= 0; i -= 4)
        {
            unsigned char nibble = (value >> i) & 0xf;
            write_char(destination, hex_digits[nibble]);
        }
    }
}

