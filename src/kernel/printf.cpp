#include "printf.hpp"

#include <interface/serial.hpp>

namespace kernel::utility
{
    print::print(hal::interface::serial &injected_serial) : _serial(injected_serial)
    {
    }

    void print::sprintf(char *buffer, const char *format, ...)
    {
        __builtin_va_list args;
        __builtin_va_start(args, format);

        char* dest = buffer;
        int chars_written = 0;

        for (int i = 0; format[i] != '\0'; i++)
        {
            if (format[i] == '%')
            {
                i++;

                if (format[i] == 'd')
                {
                    int num = __builtin_va_arg(args, int);
                    chars_written += write_int(&dest, num);
                }
            }
            else
            {
                write_char(&dest, format[i]);
                chars_written++;
            }
        }
            write_char(&dest, '\0');
            __builtin_va_end(args);
    }

    void print::write_char(char** dest, char c)
    {
        if (*dest)
        {
            **dest = c;
            (*dest)++;
        }
    }

    int print::write_int(char** dest, int num)
    {
        if (num < 0)
        {
            write_char(dest, '-');
            num = -num;
        }

        if (num == 0)
        {
            write_char(dest, '0');
            return 1;
        }

        char buffer[20];
        int len = 0;

        while (num > 0)
        {
            buffer[len++] = '0' + (num % 10);
            num /= 10;
        }

        for (int i = len - 1; i >= 0; i--)
        {
            write_char(dest, buffer[i]);
        }

        return len;
        
    }
}
