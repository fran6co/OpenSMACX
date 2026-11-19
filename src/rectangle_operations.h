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

// Descriptive recovery operation; this name does not identify an original
// entry. The four-edge setter the image expands at 0x00440923 (DiploPop),
// 0x004ADF95 (SetupWin), 0x0055AFEF (base_draw) and 0x0055A150 (veh_scoot),
// MonuWin's icon rects and QuayleWin: the edges are evaluated right to left,
// as call arguments are, then stored in order through the pointer.
// WORKAROUND(decl): the name is descriptive. Copies src into dst inset by
// the horizontal and vertical amounts, reading all four source edges through
// the source pointer (NewTechWin 0x00483FB3..0x00483FF2, DiploWin).
static inline void inset_rect(RECT *dst, const RECT *src, int horizontal,
                              int vertical) {
    dst->left = src->left + horizontal;
    dst->top = src->top + vertical;
    dst->right = src->right - horizontal;
    dst->bottom = src->bottom - vertical;
}

static inline void __cdecl set_rect(RECT *rect, int left, int top, int right,
                                    int bottom) {
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}
