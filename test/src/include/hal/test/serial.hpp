#include <hal/interface/serial.hpp>

#include <stdio.h>

class std_serial : public hal::interface::serial
{
    void init_serial(library::common::word baud_rate) override {};

    uint8_t read_serial() override
    {
        return 0;
    };

    void write_serial(char data) override
    {
        putchar(data);
    };

    void write_string_serial(char *out) override
    {
        printf("%s", out);
    };
};
