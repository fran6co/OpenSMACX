#include "stdafx.h"
#include "font.h"
#include "vector_teardown.h"

Font::Font()
    : unk_1_(-1),
      is_fot_set_(0),
      font_obj_(nullptr),
      line_height_(0),
      height_(0),
      ascent_(0),
      descent_(0),
      fot_file_name_(nullptr) {
}

Font::Font(LPSTR font_name, int height, int style) {
    init(font_name, height, static_cast<uint32_t>(style));
}

Font::~Font() {
    close();
}

// The per-element teardown the queue passes: the Font destructor, bound here
// under its own name so this file needs nothing from the generated thunk
// family. atexit_thunks.cpp binds the same address as FontElementTeardown.
func_thiscall_teardown FontQueueElementTeardown =
    original_method<func_thiscall_teardown>(0x00618EE0);

/*
Purpose: Destroy the queue: hand the three-slot walk to the CRT vector
         iterator with the queue itself as the array base and the Font
         destructor as the per-element teardown.
// ORIGINAL: 0x0055B740 ??1FontQueue@@QAE@XZ 0x0055B740-0x0055B750
// size      16 bytes
// prototype void (__thiscall ??1FontQueue@@QAE@XZ)(FontQueue* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006456E4
Return Value: n/a
Status: Complete
*/
FontQueue::~FontQueue() {
    VectorDtorIterator(this, 0x28, 3, FontQueueElementTeardown);
}

void __fastcall font_queue_dtor_redirect(FontQueue *self, void *) {
    self->~FontQueue();
}

// 0x009BB484. The process default font: `Font::init_font_class` sets it,
// `close_font_class` clears it, and Buffer, Menu and PullDown fall back to
// it - 42 references. Past `.data`'s stored bytes, so there is no
// initialiser to preserve and a real object is the whole of it.
Font *FontDefault;
int *FontSizeTable = (int *)0x0068F220;
Font *FontTable = (Font *)0x0093FC58;

/*
Purpose: Select the Font whose point size is closest to the requested one,
         then index that size's row by style.
// ORIGINAL: 0x005882F0 ?find_font@@YAHHH@Z 0x005882F0-0x0058833B
// symbol    ?find_font@@YAPAVFont@@HH@Z
// size      75 bytes
// prototype 
// callers   31   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
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

/*
Purpose: Legacy stub; the original body returns 1 without reading its
         arguments.
// ORIGINAL: 0x00618F30 ?UNK1@Font@@QAEHHHHH@Z 0x00618F30-0x00618F38 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK1@Font@@QAEHHHHH@Z)(Font* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Font::UNK1(int, int, int, int) {
    return 1;
}

int __fastcall font_unk1_redirect(
        Font *self, void *, int a, int b, int c, int d) {
    return self->UNK1(a, b, c, d);
}
