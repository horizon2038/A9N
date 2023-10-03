#include <logger.hpp>

namespace kernel::utility
{
    uint32_t logger::log_id = 0;
    logger *logger::this_logger = nullptr;

    logger::logger(hal::interface::serial &target_serial)
        : _print(target_serial)
    {
        uint8_t log_id = 0;
        this_logger = this;
        this_logger->_print.printf("\e[0m");
    }

    void logger::log(const char *sender, const char *message)
    {
        this_logger->_print.printf("\e[0m[\e[32m %d \e[0m]\e[37m\e[11G[ %s ]\e[25G: %s\n\e[0m", log_id, sender, message);
        log_id++;
    }

    void logger::debug(const char *message, const uint64_t value)
    {
        this_logger->_print.printf("\e[0m[\e[34m %d \e[0m]\e[37m\e[11G[\e[34m %s \e[0m]\e[25G: [ %d ] %s\n\e[0m", log_id, "DEBUG", value, message);
        log_id++;
    }

    void logger::error(const char *message)
    {
        this_logger->_print.printf("\e[0m[\e[31m %d \e[0m]\e[37m\e[11G[\e[31m %s \e[0m]\e[25G: %s\n\e[0m", log_id, "ERROR", message);
        while(1);
        log_id++;
    }

    void logger::a9nout()
    {
        this_logger->_print.printf
        (
            "\e[32m" \
            "                                   \n" \
            "        d8888 .d8888b. 888b    888 \n" \
            "       d88888d88P  Y88b8888b   888 \n" \
            "      d88P888888    88888888b  888 \n" \
            "     d88P 888Y88b. d888888Y88b 888 \n" \
            "    d88P  888  Y888P888888 Y88b888 \n" \
            "   d88P   888       888888  Y88888 \n" \
            "  d8888888888Y88b  d88P888   Y8888 \n" \
            " d88P     888  Y8888P  888    Y888 \n" \
            "                                   \n" \
            "\e[0m"
        );

        this_logger->_print.printf
        (
            "\e[32m" \
            "                                   \n" \
            "         @@@@@@  @@@@@@@@@         \n" \
            "        @@@@@@@  @@@@@@@@@@        \n" \
            "        @@@@@@  @@@@@@@@@@@        \n" \
            "       @@@@@@@  @@@@@@@@@@@@       \n" \
            "       @@@@@@  @@@@@@ @@@@@@       \n" \
            "      @@@@@@@  @@@@@@ &@@@@@@      \n" \
            "      @@@@@@  @@@@@@   @@@@@@      \n" \
            "     @@@@@@@  @@@@@@   0@@@@@@     \n" \
            "     @@@@@@  @@@@@@  @  @@@@@@     \n" \
            "    @@@@@@&  @@@@@@  @@ &@@@@@@    \n" \
            "    @@@@@@  ......  @@@  @@@@@@    \n" \
            "   @@@@@@@  @@@@@@@@@@@@ &@@@@@@   \n" \
            "   @@@@@@  @@@@@@@@@@@@@  @@@@@@   \n" \
            "  @@@@@@@ .@@@@@@@@@@@@@0 (@@@@@@  \n" \
            "  @@@@@@  @@@@@@           @@@@@@  \n" \
            " @@@@@@0  @@@@@@           /@@@@@@ \n" \
            "                                   \n" \
            "\e[0m"
        );

        this_logger->_print.printf
        (
            "\e[11F" \
            "\e[36G" \
            "\e[32m" \
            "kernel: \e[52G\e[37mA9N v0.0.1\n" \
            "\e[36G" \
            "\e[32m" \
            "architecture: \e[52G\e[37m%s\n" \
            "\e[36G" \
            "\e[32m" \
            "creator: \e[52G\e[37mhorizon2k38\n" \
            "\e[36G" \
            "\e[32m" \
            "project: \e[52G\e[37mmitoujr\n" \
            "\e[36G" \
            "\e[32m" \
            "about: \e[52G\e[37mA9N is a kernel built on HAL <Hardware Abstraction Layer> and microkernel.\n" \
            "\e[36G" \
            "\e[32m" \
            "\e[52G\e[37mIt combines high portability, stability, and scalability.\n" \
            "\e[0m" \
            "\e[13E", "x86_64" // TODO: change hard-coded architexture.
        );
        
    }
}

