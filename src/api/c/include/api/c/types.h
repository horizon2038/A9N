#ifndef A9N_API_C_TYPES_H
#define A9N_API_C_TYPES_H

#include <stdint.h>

typedef uintmax_t a9n_word;

typedef a9n_word a9n_capability_descriptor;
typedef a9n_word a9n_capability_tag;

#define A9N_BYTE_BITS 8
#define A9N_WORD_BITS (sizeof(a9n_word))

#endif
