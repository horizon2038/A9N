#ifndef A9N_APIC_C_KERNEL_CALL_X86_64_H
#define A9N_APIC_C_KERNEL_CALL_X86_64_H

#include <api/c/types.h>

void a9n_kernel_call(
    a9n_capability_descriptor descriptor,
    a9n_word                  depth,
    a9n_word                  tag,
    a9n_word                  message_length
);

#endif
