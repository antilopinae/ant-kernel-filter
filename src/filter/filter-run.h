#pragma once

#include <stddef.h>

namespace ant::filter {

typedef struct {
    const char* name;
    void*       ptr;
    int         is_input;
} ant_ExtPort;

}
