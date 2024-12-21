#include <kernel/utility/logger.hpp>

namespace a9n::kernel::utility
{
    uint32_t logger::log_id      = 0;
    logger  *logger::this_logger = nullptr;

    using guard                  = lock_guard<spin_lock_no_owner>;

    logger::logger(a9n::hal::serial &target_serial) : _print(target_serial)
    {
        this_logger = this;
        this_logger->_print.printf("\e[0m");
    }

    void logger::log(const char *sender, const char *message)
    {
        guard g { this_logger->lock };
        this_logger->print_log_id();
        this_logger->print_sender(sender);
        this_logger->print_splitter();
        this_logger->_print.printf("%s\n", message);
    }

    void logger::debug(const char *message, ...)
    {
        guard             g { this_logger->lock };
        __builtin_va_list args;
        __builtin_va_start(args, message);
        this_logger->print_log_id(terminal_color::CYAN);
        this_logger->print_sender("DEBUG", terminal_color::CYAN);
        this_logger->print_splitter();
        this_logger->_print.vprintf(message, args);
        this_logger->_print.printf("\n");
        __builtin_va_end(args);
    }

    void logger::error(const char *message)
    {
        guard g { this_logger->lock };
        this_logger->print_log_id();
        this_logger->print_sender("ERROR", terminal_color::RED);
        // this_logger->print_splitter();
        this_logger->_print.printf("%s\n", message);
    }

    void logger::printk(const char *format, ...)
    {
        guard             g { this_logger->lock };
        __builtin_va_list args;
        __builtin_va_start(args, format);
        this_logger->print_log_id(terminal_color::GREEN);
        this_logger->print_sender("KERNEL");
        this_logger->print_splitter();
        this_logger->_print.vprintf(format, args);
        __builtin_va_end(args);
    }

    void logger::printh(const char *format, ...)
    {
        guard             g { this_logger->lock };
        __builtin_va_list args;
        __builtin_va_start(args, format);
        this_logger->print_log_id(terminal_color::YELLOW);
        this_logger->print_sender("HAL");
        this_logger->print_splitter();
        this_logger->_print.vprintf(format, args);
        __builtin_va_end(args);
    }

    void logger::printu(const char *format, ...)
    {
        __builtin_va_list args;
        __builtin_va_start(args, format);
        this_logger->print_log_id(terminal_color::YELLOW);
        this_logger->print_sender("USER");
        this_logger->print_splitter();
        this_logger->_print.vprintf(format, args);
        __builtin_va_end(args);
    }

    void logger::printn(const char *format, ...)
    {
        __builtin_va_list args;
        __builtin_va_start(args, format);
        this_logger->_print.vprintf(format, args);
        __builtin_va_end(args);
    }

    void logger::put_char(char target)
    {
        this_logger->_print.put_char(target);
    }

    void logger::split()
    {
        this_logger->_print.printf("\n");
    }

    void logger::a9nout()
    {
        this_logger->_print.printf(
            "\e[32m"
            "                                   \n"
            "        d8888 .d8888b. 888b    888 \n"
            "       d88888d88P  Y88b8888b   888 \n"
            "      d88P888888    88888888b  888 \n"
            "     d88P 888Y88b. d888888Y88b 888 \n"
            "    d88P  888  Y888P888888 Y88b888 \n"
            "   d88P   888       888888  Y88888 \n"
            "  d8888888888Y88b  d88P888   Y8888 \n"
            " d88P     888  Y8888P  888    Y888 \n"
            "                                   \n"
            "\e[0m"
        );

        this_logger->_print.printf(
            "\e[32m"
            "                                   \n"
            "         @@@@@@  @@@@@@@@@         \n"
            "        @@@@@@@  @@@@@@@@@@        \n"
            "        @@@@@@  @@@@@@@@@@@        \n"
            "       @@@@@@@  @@@@@@@@@@@@       \n"
            "       @@@@@@  @@@@@@ @@@@@@       \n"
            "      @@@@@@@  @@@@@@ &@@@@@@      \n"
            "      @@@@@@  @@@@@@   @@@@@@      \n"
            "     @@@@@@@  @@@@@@   0@@@@@@     \n"
            "     @@@@@@  @@@@@@  @  @@@@@@     \n"
            "    @@@@@@&  @@@@@@  @@ &@@@@@@    \n"
            "    @@@@@@  ......  @@@  @@@@@@    \n"
            "   @@@@@@@  @@@@@@@@@@@@ &@@@@@@   \n"
            "   @@@@@@  @@@@@@@@@@@@@  @@@@@@   \n"
            "  @@@@@@@ .@@@@@@@@@@@@@0 (@@@@@@  \n"
            "  @@@@@@  @@@@@@           @@@@@@  \n"
            " @@@@@@0  @@@@@@           /@@@@@@ \n"
            "                                   \n"
            "\e[0m"
        );

        this_logger->_print.printf(
            "\e[11F"
            "\e[36G"
            "\e[32m"
            "kernel: \e[52G\e[37mA9N v0.0.1\n"
            "\e[36G"
            "\e[32m"
            "architecture: \e[52G\e[37m%s\n"
            "\e[36G"
            "\e[32m"
            "creator: \e[52G\e[37mhorizon2k38\n"
            "\e[36G"
            "\e[32m"
            "project: \e[52G\e[37mmitoujr\n"
            "\e[36G"
            "\e[32m"
            "about: \e[52G\e[37mA9N is a kernel built on HAL <Hardware "
            "Abstraction Layer> and microkernel.\n"
            "\e[36G"
            "\e[32m"
            "\e[52G\e[37mIt combines high portability, stability, and "
            "scalability.\n"
            "\e[0m"
            "\e[13E",
            "x86_64" // TODO: change hard-coded architecture.
        );
    }

    void logger::print_log_id(const char *color_id)
    {
        this_logger->_print
            .printf("[ %s%s%010d%s ] ", terminal_color::RESET, color_id, log_id, terminal_color::RESET);
        log_id++;
    }

    void logger::print_sender(const char *sender, const char *color_id)
    {
        this_logger->_print
            .printf("[ %s%s%8s%s ] ", terminal_color::RESET, color_id, sender, terminal_color::RESET);
    }

    void logger::print_splitter()
    {
        this_logger->_print.printf(": ");
    }

    void logger::mitoujr()
    {
        this_logger->_print.printf(
            "\e[32m"
            "                                                  \n"
            "                         /                        \n"
            "                       /////                      \n"
            "                      /// ///                     \n"
            "                    ///     ///                   \n"
            "                   ///       ///                  \n"
            "                 ////         ////                \n"
            "                ////           ////               \n"
            "              ////////       ////////             \n"
            "             ///    ///     ///    ///            \n"
            "            ///      ///   ///      ///           \n"
            "          ///          /////          ///         \n"
            "         ///            ///            ///        \n"
            "       ///            /// ///            ///      \n"
            "      ///            ///   ///            ///     \n"
            "    ////           ////     ////           ////   \n"
            "   ///            ///         ///            ///  \n"
            "  //////////////////           //////////////////(\n"
            "                                                  \n"
            "\e[0m"
        );
    }
}
