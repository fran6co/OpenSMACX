#pragma once

#include <windows.h>

// Descriptive recovery operation; this name does not identify an original entry.
static inline RECT *__cdecl set_rectangle_dimensions(
    RECT *rect, int x, int y, int width, int height) {
    // PLAIN ASSIGNMENTS. The `volatile RECT *` and the `long_from_bits`
    // bit-cast were written to pin the store order and the signedness; both
    // are things VC6 cannot fold, and together they doubled the body. The
    // image's order IS declaration order, so nothing had to be pinned.
    rect->left = x;
    rect->top = y;
    rect->right = x + width;
    rect->bottom = y + height;
    return rect;
}
