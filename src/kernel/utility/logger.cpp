#include "hal/hal_result.hpp"
#include "kernel/kernel_result.hpp"
#include <kernel/utility/logger.hpp>
#include <kernel/version.hpp>

#include <hal/interface/cpu.hpp>

namespace a9n::kernel::utility
{
    uint32_t logger::log_id      = 0;
    logger  *logger::this_logger = nullptr;

    using guard                  = lock_guard<spin_lock_no_owner>;

    logger::logger()
    {
        this_logger = this;
        this_logger->_print.printf("\e[0m");
    }

    void logger::log(const char *sender, const char *message)
    {
        guard g { this_logger->lock };
        this_logger->print_log_id();
        this_logger->print_sender(sender);
        this_logger->print_core();
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
        this_logger->print_core();
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
        this_logger->print_core();
        this_logger->print_splitter();
        this_logger->_print.printf("%s\n", message);
    }

    void logger::printk(const char *format, ...)
    {
        guard             g { this_logger->lock };
        __builtin_va_list args;
        __builtin_va_start(args, format);
        this_logger->print_log_id(terminal_color::GREEN);
        this_logger->print_sender("KERNEL", terminal_color::GREEN);
        this_logger->print_core();
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
        this_logger->print_sender("HAL", terminal_color::YELLOW);
        this_logger->print_core();
        this_logger->print_splitter();
        this_logger->_print.vprintf(format, args);
        __builtin_va_end(args);
    }

    void logger::printu(const char *format, ...)
    {
        guard             g { this_logger->lock };
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
        guard             g { this_logger->lock };
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

    // clang-format off
    constexpr const char *A9N_LOGO_ASCII[] = {
        "                                   ",
        "         @@@@@@  @@@@@@@@@         ",
        "        @@@@@@@  @@@@@@@@@@        ",
        "        @@@@@@  @@@@@@@@@@@        ",
        "       @@@@@@@  @@@@@@@@@@@@       ",
        "       @@@@@@  @@@@@@ @@@@@@       ",
        "      @@@@@@@  @@@@@@ &@@@@@@      ",
        "      @@@@@@  @@@@@@   @@@@@@      ",
        "     @@@@@@@  @@@@@@   0@@@@@@     ",
        "     @@@@@@  @@@@@@  @  @@@@@@     ",
        "    @@@@@@&  @@@@@@  @@ &@@@@@@    ",
        "    @@@@@@  ......  @@@  @@@@@@    ",
        "   @@@@@@@  @@@@@@@@@@@@ &@@@@@@   ",
        "   @@@@@@  @@@@@@@@@@@@@  @@@@@@   ",
        "  @@@@@@@ .@@@@@@@@@@@@@0 (@@@@@@  ",
        "  @@@@@@  @@@@@@           @@@@@@  ",
        " @@@@@@0  @@@@@@           /@@@@@@ ",
        "                                   "
    };
    constexpr const char *A9N_LOGO_ASCII_STR[] = {
        "                                   ",
            "        d8888 .d8888b. 888b    888 ",
            "       d88888d88P  Y88b8888b   888 ",
            "      d88P888888    88888888b  888 ",
            "     d88P 888Y88b. d888888Y88b 888 ",
            "    d88P  888  Y888P888888 Y88b888 ",
            "   d88P   888       888888  Y88888 ",
            "  d8888888888Y88b  d88P888   Y8888 ",
            " d88P     888  Y8888P  888    Y888 ",
            "                                   "
    };
    constexpr const char A9N_DESCRIPTION[] = "A9N is a Capability-based Microkernel that supports a variety of hardware platforms through appropriate HAL.";
    constexpr const char A9N_AUTHOR[]      = "Rekka 'horizon' IGUMI";
    constexpr const char A9N_PROJECT[]     = "A9N Project";
    // clang-format on

    void logger::a9nout()
    {
        for (const char *line : A9N_LOGO_ASCII)
        {
            printk("\e[32m%s\e[0m\n", line);
        }

        printk("A9N Microkernel v%s\n", KERNEL_VERSION_STRING);
        printk("  Description: %s\n", A9N_DESCRIPTION);
        printk("  Project: %s\n", A9N_PROJECT);
        printk("  Author: %s\n", A9N_AUTHOR);
        printk("\n");
    }

    void logger::print_log_id([[maybe_unused]] const char *color_id)
    {
#ifdef A9N_CONFIG_ENABLE_LOG_ID
        this_logger->_print
            .printf("[ %s%s%010d%s ]", terminal_color::RESET, color_id, log_id, terminal_color::RESET);
        log_id++;
#endif
    }

    void logger::print_sender(const char *sender, const char *color_id)
    {
        this_logger->_print
            .printf("[%s%s%8s%s]", terminal_color::RESET, color_id, sender, terminal_color::RESET);
    }

    void logger::print_core(void)
    {
#ifdef A9N_CONFIG_ENABLE_LOG_CORE_ID
        auto result
            = a9n::hal::current_core_number()
                  .and_then(
                      [](a9n::word core_number) -> a9n::hal::hal_result
                      {
                          this_logger->_print.printf("[%02x]", core_number);

                          return {};
                      }
                  )
                  .transform_error(convert_hal_to_kernel_error)
                  .or_else(
                      [](kernel_error e) -> kernel_result
                      {
                          this_logger->_print.printf("[%02s]", "-");

                          return {};
                      }
                  );
#endif
    }

    void logger::print_splitter()
    {
        this_logger->_print.printf(" ");
    }
}
