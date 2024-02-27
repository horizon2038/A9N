#ifndef _LIBCPP_NEW
#define _LIBCPP_NEW

#include <stddef.h>

void *operator new(size_t, void *) throw();
void *operator new[](size_t, void *) throw();
void operator delete(void *) throw();
void operator delete[](void *, void *) throw();

#endif
