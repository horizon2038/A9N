#ifndef PRINT_HPP
#define PRINT_HPP

#include <hal/interface/serial.hpp>

namespace kernel::utility
{
    class print
    {
      public:
        print(hal::interface::serial &injected_serial);

        void printf(const char *format, ...);
        void vprintf(const char *format, __builtin_va_list args);

        void sprintf(char *buffer, const char *format, ...);
        void vsprintf(char *buffer, const char *format, __builtin_va_list args);

      private:
        hal::interface::serial &_serial;

        void process_format(
            char            **destination,
            const char      **format_pointer,
            __builtin_va_list args
        );
        void write_char(char **destination, char target_char);
        void
            write_string(char **destination, char *target_string, int width = 0);
        void write_int(
            char **destination,
            int    count,
            int    width    = 0,
            bool   zero_pad = false
        );
        void write_int_ll(
            char    **destination,
            long long count,
            int       width    = 0,
            bool      zero_pad = false
        );
        void write_uint(
            char       **destination,
            unsigned int count,
            int          width,
            bool         zero_pad
        );
        void write_uint_ll(
            char             **destination,
            unsigned long long count,
            int                width,
            bool               zero_pad
        );
        void write_hex(
            char       **destination,
            unsigned int count,
            int          width     = 0,
            bool         zero_pad  = false,
            bool         uppercase = false
        );
        void write_hex_ll(
            char             **destination,
            unsigned long long count,
            int                width,
            bool               zero_pad,
            bool               uppercase = false
        );
        void write_pointer(char **destination, const void *pointer);

        char print_buffer[1024];
    };
}

#endif
