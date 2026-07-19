#include "stdafx.h"
#include "font.h"

Font::Font()
    : unk_1_(-1),
      is_fot_set_(0),
      font_obj_(nullptr),
      line_height_(0),
      height_(0),
      ascent_(0),
      descent_(0),
      fot_file_name_(nullptr) {
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Font::Font(LPSTR font_name, int height, int style) {
    init(font_name, height, static_cast<uint32_t>(style));
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

Font::~Font() {
    close();
}

Font **FontDefaultPtr = (Font **)0x009BB484;
int *FontSizeTable = (int *)0x0068F220;
Font *FontTable = (Font *)0x0093FC58;

/*
Purpose: Select the Font whose point size is closest to the requested one,
         then index that size's row by style.
Original Offset: 005882F0
Status: Complete
*/
Font *__cdecl find_font(int size, int style) {
    int best_index = 0;
    // The seed doubles as a threshold: a requested size further than 9999 from
    // every candidate matches nothing and falls back to the first entry.
    int best_delta = 9999;
    for (size_t index = 0; index < FontSizeTableCount; ++index) {
        const int candidate = FontSizeTable[index];
        // Absolute difference, branching on sign exactly as the legacy body.
        int delta = candidate - size;
        if (delta < 0) {
            delta = size - candidate;
        }
        if (delta < best_delta) {
            best_delta = delta;
            best_index = static_cast<int>(index);
        }
    }
    // Each size occupies four consecutive style slots in the table.
    return FontTable + (style + best_index * 4);
}
