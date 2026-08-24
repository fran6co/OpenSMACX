// ORIGINAL: 0x005DDAB0 ?wrap_cent@Buffer@@QAEHPADPAURECT@@@Z 0x005DDAB0-0x005DDBA6 FILE BYTE_EXACT
// size      246 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPADPAURECT@@@Z)(Buffer* this, char*, RECT*)
// callers   ?   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0
//
// LOAD-BEARING SOURCE FORM - the logic below was already right at MISMATCH
// #0; four spelling changes, none of them semantic, carried it to BYTE_EXACT.
// Each was measured on its own, so the next reader can trust the order:
//
//   #0  -> #22  `#pragma function(strlen)`.  /O2 expands `strlen` to
//               `repne scasb`, which burns EDI and ECX inside the loop and
//               costs the register allocator EBP; the original calls
//               0x006453E0.
//   #22 -> #39  `int lineWidth = width;` at the TOP of the loop body rather
//               than inside the non-null arm.  This is what frees EBP: with
//               the copy in the arm, /O2 keeps an EBP frame and spills
//               `width`; hoisted, EBP holds `width` and the frame is the
//               original's one-slot `push ecx`.
//   #39 -> #61  the `write_cent_l` CALL DUPLICATED into both arms instead of
//               a shared call taking a `chunkLen` temporary.  VC6 tail-merges
//               the common `push str; mov ecx,this; call` suffix and leaves
//               the four argument pushes duplicated, which is the original's
//               shape.
//   #61 -> BYTE_EXACT  real class members (`lineCount_`, `font1_`) instead of
//               `reinterpret_cast` through `(char *)this + 0x18` / `+ 0x52c`.
//               Field accesses let VC6 prove the two do not alias, so it
//               hoists the `font1_` load above the `lineCount_` store the way
//               the original schedules it.  The cast form cannot express that.
//
// Also measured, and NOT levers: locals-at-top, locals-at-first-use, no named
// locals at all, and declaring `lineWidth` before or after the other locals -
// all four sat at exactly the divergence of the shape they were built on.

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes carry
// only the members this body touches and globals are declared, not defined.

struct RECT {
    int left;
    int top;
    int right;
    int bottom;
};

// Offsets and names as src/recovered/units/005e10c0.cpp already spells them.
struct Font {
    int unk_1_;         // 0x00  negative selects line_height_ below
    int is_fot_set_;    // 0x04
    void *font_obj_;    // 0x08
    int line_height_;   // 0x0C
    int height_;        // 0x10
};

class Buffer {
public:
    char reserved_0_[0x18];
    int lineCount_;                       // 0x18  lines emitted by this wrap
    char reserved_1C_[0x52C - 0x1C];
    Font *font1_;                         // 0x52C

    int find_line_break_l(char *, int *, int);
    int write_cent_l(char *, int, int, int, int);
    int wrap_cent(char *, RECT *);
};

extern "C" unsigned int strlen(const char *);

// The original CALLS strlen at 0x006453E0; /O2 would otherwise inline it.
#pragma function(strlen)

extern int &BufferStrHeight;
extern Font *g_009bb484;

// Lays `text` out centred inside `rect`, one line per call to write_cent_l,
// and returns the y coordinate just past the last line.
int Buffer::wrap_cent(char * text, RECT * rect) {
    if (text == 0 || rect == 0) {
        return rect->top;
    }
    &BufferStrHeight = 0;
    this->lineCount_ = 0;
    int left = rect->left;
    int width = rect->right - left;
    char *str = text;
    int yPos = rect->top;
    char *nextStr;
    for (;;) {
        int lineWidth = width;
        if (str == 0) {
            nextStr = 0;
        } else {
            &BufferStrHeight = 0;
            unsigned int fullLen = strlen(str);
            nextStr = reinterpret_cast<char *>(
                find_line_break_l(str, &lineWidth, static_cast<int>(fullLen)));
            if (nextStr != 0) {
                write_cent_l(str, left, yPos, width,
                             static_cast<int>(nextStr - str));
            } else {
                write_cent_l(str, left, yPos, width,
                             static_cast<int>(strlen(str)));
            }
        }
        this->lineCount_ = this->lineCount_ + 1;
        if (this->font1_ == 0) {
            this->font1_ = g_009bb484;
        }
        Font *font = this->font1_;
        int advance;
        if (font->unk_1_ >= 0) {
            advance = font->height_ + font->unk_1_;
        } else {
            advance = font->line_height_;
        }
        yPos = yPos + advance;
        str = nextStr;
        if (nextStr == 0) {
            break;
        }
    }
    return yPos;
}
