#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdint.h>
#include <kernel/utility/print.hpp>
#include <hal/interface/serial.hpp>

namespace a9n::kernel::utility
{
    enum class log_type : uint8_t
    {
        INFO,
        INIT,
        RUN,
        QUIT,
        START,
        STOP,
        ENABLE,
        DISABLE
    };

    namespace terminal_color
    {
        const static char *RESET      = "\e[0m";
        const static char *BLACK      = "\e[30m";
        const static char *BLACK_BG   = "\e[40m";
        const static char *RED        = "\e[31m";
        const static char *RED_BG     = "\e[41m";
        const static char *GREEN      = "\e[32m";
        const static char *GREEN_BG   = "\e[42m";
        const static char *YELLOW     = "\e[33m";
        const static char *YELLOW_BG  = "\e[43m";
        const static char *BLUE       = "\e[34m";
        const static char *BLUE_BG    = "\e[44m";
        const static char *MAGENTA    = "\e[35m";
        const static char *MAGENTA_BG = "\e[45m";
        const static char *CYAN       = "\e[36m";
        const static char *CYAN_BG    = "\e[46m";
        const static char *WHITE      = "\e[37m";
        const static char *WHITE_BG   = "\e[47m";
    }

    class logger
    {
      public:
        logger(a9n::hal::serial &target_serial);
        ~logger();

        static void log(const char *sender, const char *message);
        static void debug(const char *message, ...);
        static void error(const char *message);
        static void printk(const char *format, ...);
        static void printn(const char *format, ...);
        static void split(); // for clean-log
        static void a9nout();

        static void mitoujr();

      private:
        static uint32_t log_id;
        static logger  *this_logger;
        print           _print;

        void print_log_id(const char *color_id = terminal_color::GREEN);
        void print_sender(
            const char *sender,
            const char *color_id = terminal_color::RESET
        );
        void print_splitter();
    };

}

#endif
