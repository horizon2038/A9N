#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <stdint.h>

struct message
{
    int32_t sender_process_id;
    char messagge_type[64];
    uint8_t data[1024];
};

#endif

