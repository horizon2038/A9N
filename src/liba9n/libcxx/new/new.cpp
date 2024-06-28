#include <liba9n/libcxx/new>

void *operator new([[maybe_unused]] size_t size, void *buffer) throw()
{
    return buffer;
}

void *operator new[]([[maybe_unused]] size_t size, void *buffer) throw()
{
    return buffer;
}

void operator delete([[maybe_unused]] void *object) throw()
{
    // delete object.
}

void operator delete[](
    [[maybe_unused]] void *object,
    [[maybe_unused]] void *buffer
) throw()
{
}
