#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <hal/interface/serial.hpp>
#include <kernel/utility/print.hpp>
#include <stdint.h>

#include <kernel/process/lock.hpp>

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
        static const char *RESET      = "\e[0m";
        static const char *BLACK      = "\e[30m";
        static const char *BLACK_BG   = "\e[40m";
        static const char *RED        = "\e[31m";
        static const char *RED_BG     = "\e[41m";
        static const char *GREEN      = "\e[32m";
        static const char *GREEN_BG   = "\e[42m";
        static const char *YELLOW     = "\e[33m";
        static const char *YELLOW_BG  = "\e[43m";
        static const char *BLUE       = "\e[34m";
        static const char *BLUE_BG    = "\e[44m";
        static const char *MAGENTA    = "\e[35m";
        static const char *MAGENTA_BG = "\e[45m";
        static const char *CYAN       = "\e[36m";
        static const char *CYAN_BG    = "\e[46m";
        static const char *WHITE      = "\e[37m";
        static const char *WHITE_BG   = "\e[47m";
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
        static void printh(const char *format, ...);
        static void printu(const char *format, ...);
        static void printn(const char *format, ...);
        static void put_char(char target);
        static void split(); // for clean-log
        static void a9nout();

        static void mitoujr();

      private:
        static uint32_t    log_id;
        static logger     *this_logger;
        print              _print;
        spin_lock_no_owner lock;

        void print_log_id(const char *color_id = terminal_color::GREEN);
        void print_sender(const char *sender, const char *color_id = terminal_color::RESET);
        void print_splitter();
    };

}

#endif
