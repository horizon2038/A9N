#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdint.h>
#include "../print.hpp"
#include "interface/serial.hpp"

namespace kernel::utility
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

    class logger
    {
        public:
            logger(hal::interface::serial &target_serial);
            ~logger();

            static void log(const char *sender, const char *message); 
            static void debug(const char *message, const uint64_t value); 
            static void error(const char *message); 
        
        private:
            static uint32_t log_id;
            static logger *this_logger; 
            print _print;
    };

}

#endif
