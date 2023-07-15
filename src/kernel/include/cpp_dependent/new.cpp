#include <stddef.h>

void* operator new(size_t size, void *buffer)
{
    return buffer;
}

void operator delete(void *object) noexcept
{
    // delete object.
}

// question: can i use stddef.h in -ffreestanding operator ?
// 