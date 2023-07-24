#include <cpp_dependent/new.hpp>

void* operator new(size_t size, void *buffer) throw()
{
    return buffer;
}

void* operator new[](size_t size, void *buffer) throw()
{
    return buffer;
}

void operator delete(void *object, void *buffer) throw()
{
    // delete object.
}

void operator delete[](void *object, void *buffer) throw()
{

}

// question: can i use stddef.h in -ffreestanding operator ?
// 
