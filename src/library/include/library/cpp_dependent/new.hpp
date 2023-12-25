#ifndef CPPNEW
#define CPPNEW

#include <stddef.h>

void *operator new(size_t, void *) throw();
void *operator new[](size_t, void *) throw();
void operator delete(void *, void *) throw();
void operator delete[](void *, void *) throw();

#endif
