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
}

