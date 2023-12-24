#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <stdint.h>

struct message
{
    int32_t sender_process_id;
    int32_t type;
    uint8_t data[1024];
};

#endif
