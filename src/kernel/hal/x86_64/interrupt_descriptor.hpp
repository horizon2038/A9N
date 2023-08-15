#ifndef INTERRUPT_DESCRIPTOR_H
#define INTERRUPT_DESCRIPTOR_H

#include <stdint.h>

typedef struct __attribute__((packed))
{
   uint16_t offset_low;
   uint16_t kernel_cs;
   uint8_t ist;
   uint8_t type;
   uint16_t offset_mid;
   uint32_t offset_high;
   uint32_t reserved;
} interrupt_descriptor_64;

constexpr static uint8_t INTERRUPT_GATE = 0xe;
constexpr static uint8_t PRESENT = 0x80;

#endif
