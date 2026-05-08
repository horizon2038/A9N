#include <api/c/arch/x86_64/kernel_call.h>

// asm
extern "C" void _a9n_kernel_call(
    a9n_capability_descriptor descriptor,
    a9n_word                  depth,
    a9n_word                  tag,
    a9n_word                  message_length
);

void a9n_kernel_call(
    a9n_capability_descriptor descriptor,
    a9n_word                  descriptor_depth,
    a9n_word                  message_tag,
    a9n_word                  message_length
)
{
    _a9n_kernel_call(descriptor, descriptor_depth, message_tag, message_length);
}
