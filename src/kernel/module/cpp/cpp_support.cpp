/*
* Kernel C++ support
 */

#include <cstddef>
#include "kern_lib.h"
#include "logger.h"

void *operator new(size_t sz) throw ()
{
    return kcmalloc(sz);
}

void *operator new[](size_t sz) throw ()
{
    return kcmalloc(sz);
}

void operator delete(void *p)
{
    kcfree(p);
}

void operator delete[](void *p)
{
    kcfree(p);
}

void terminate()
{
    kern_log("terminate requested\n");
}

extern "C" void __cxa_pure_virtual()
{
    kern_log("cxa_pure_virtual error handler\n");
}