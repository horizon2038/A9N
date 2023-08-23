#include <interface/serial.hpp>

namespace kernel::utility
{
    class print
    {
        public:
            print(hal::interface::serial &injected_serial);
            void sprintf(char *buffer, const char *format, ...);
        
        private:
            hal::interface::serial &_serial;
            void write_char(char** destination, char c);
            int write_int(char** destination, int num);
    };
}

