#ifndef KERNEL_JUMPER_H
#define KERNEL_JUMPER_H

#include "stdint.h"
#include "boot_info.h"

void jump_kernel(uint64_t, boot_info *);

#endif
