// ORIGINAL: 0x005DE1D0 ?wrap_flying_flush@Buffer@@QAEHXZ 0x005DE1D0-0x005DE2DE
// RULED-OUT: the `_strlen` no-arg-extern lever (0x006141C0) moved the divergence from instruction 0 to instruction 5. The original reads `g_009b3a90`/`g_009b3a94` into ebx/ebp up front, before the text/font-metric checks that guard the call using them; this source form evaluates them only at the call site, which is a hoisting choice VC6 made differently. Not chased further.
// size      270 bytes
// prototype int (__thiscall ?wrap_flying_flush@Buffer@@QAEHXZ)(Buffer* this)
// callers   3   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DCAE0 0x006453E0

extern "C" int __cdecl _strlen();
typedef unsigned int (__cdecl *StrlenFn)(const char *);
#define strlen_(s) (reinterpret_cast<StrlenFn>(_strlen)(s))

static char *const g_009b33cc = (char *)0x009B33CC;

int Buffer::wrap_flying_flush() {
    if (*g_009b33cc != '\0') {
        unsigned int len1 = strlen_(g_009b33cc);
        if (font1_ != 0 && font1_->font_obj_ != 0) {
            unsigned int len2 = strlen_(g_009b33cc);
            unsigned int chosen = len1;
            if ((int)len2 < (int)len1) {
                chosen = strlen_(g_009b33cc);
            }
            if ((int)chosen >= 0) {
                unsigned int len3 = strlen_(g_009b33cc);
                unsigned int finalLen = len1;
                if ((int)len3 < (int)len1) {
                    finalLen = strlen_(g_009b33cc);
                }
                if (finalLen != 0) {
                    write_multi_font_raw_l(g_009b33cc, *g_009b3a90, *g_009b3a94, finalLen);
                }
            }
        }
        if (font1_ == 0) {
            font1_ = (Font *)*g_009bb484;
        }
        Font *fnt = font1_;
        int lineAdvance = (fnt->unk_1_ < 0) ? fnt->line_height_ : fnt->height_ + fnt->unk_1_;
        *g_009b3a94 = *g_009b3a94 + lineAdvance;
        field_18_ = field_18_ + 1;
    }
    *g_009b33cc = 0;
    *g_009b3a98 = 0;
    field_510_ = 0;
    field_518_ = 0;
    field_514_ = 0;
    field_51C_ = 0;
    return *g_009b3a94;
}
