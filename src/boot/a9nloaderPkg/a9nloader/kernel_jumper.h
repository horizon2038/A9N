#ifndef KERNEL_JUMPER_H
#define KERNEL_JUMPER_H

#include "boot_info.h"
#include "stdint.h"

void jump_kernel(uint64_t, boot_info *);

#endif
