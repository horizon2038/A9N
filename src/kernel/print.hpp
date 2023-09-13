#include <interface/serial.hpp>

namespace kernel::utility
{
    class print
    {
        public:
            print(hal::interface::serial &injected_serial);
            void printf(const char *format, ...);
            void sprintf(char *buffer, const char *format, ...);
            void vsprintf(char *buffer, const char *format, __builtin_va_list args);
        
        private:
            hal::interface::serial &_serial;
            void write_char(char** destination, char c);
            int write_int(char** destination, int num);
            char print_buffer[100];
    };
}

