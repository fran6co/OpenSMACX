#pragma once

#include <stdlib.h>

inline unsigned int __cdecl bounded_crt_random(int bounds) {
    if (bounds - 1 <= 0) {
        return 0;
    }
    return rand() % bounds;
}
