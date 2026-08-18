// ORIGINAL: 0x005DDDE0 ?wrap_height_flying@Buffer@@QAEHPAD@Z 0x005DDDE0-0x005DDEEA
// RULED-OUT: the `_strlen` no-arg-extern + function-pointer-cast idiom (see 0x006141C0) to avoid /O2's strlen intrinsic did not move the divergence - it is register-allocation, not a call shape. The original pushes ebx/esi/edi all in the shared prologue, before the `param_2==0` early return; this source form (an `if` straight after the first branch) defers the `push edi` past that early return since edi is unused on it. Not chased further; landing the closest structural match.
// size      266 bytes
// prototype int (__thiscall ?wrap_height_flying@Buffer@@QAEHPAD@Z)(Buffer* this, int8*)
// callers   3   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DC7C0 0x006453E0

int Buffer::wrap_height_flying(char * a1) {
    if (*g_009b3a98 == 0) {
        return 0;
    }
    if (a1 != 0) {
        if (font1_ == 0) {
            font1_ = (Font *)*g_009bb484;
        }
        char *text = a1;
        if (*text != '\0') {
            do {
                int remaining = *g_009b3a98 - *g_009b3a9c;
                text = (char *)find_line_break_l(text, &remaining, strlen(text));
                if (text == 0) {
                    break;
                }
                field_18_ = field_18_ + 1;
                if (font1_ == 0) {
                    font1_ = (Font *)*g_009bb484;
                }
                Font *fnt = font1_;
                int lineAdvance;
                if (fnt->unk_1_ < 0) {
                    lineAdvance = fnt->line_height_;
                } else {
                    lineAdvance = fnt->height_ + fnt->unk_1_;
                }
                *g_009b3a9c = 0;
                *g_009b3a94 = *g_009b3a94 + lineAdvance;
                if (field_14_ != 0) {
                    *g_009b3a98 = *g_009b3a98 - field_14_;
                    field_14_ = 0;
                }
            } while (true);
            char *prompt = (char *)0x00696CD8;
            if (prompt != 0) {
                unsigned int len = strlen(prompt);
                int width = text_width(prompt, (int)len);
                *g_009b3a9c = *g_009b3a9c + width;
            }
        }
    }
    return *g_009b3a94;
}
