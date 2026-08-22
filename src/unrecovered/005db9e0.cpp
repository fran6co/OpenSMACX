// ORIGINAL: 0x005DB9E0 ?wrap_cent@Buffer@@QAEHPAVFont@@PADHHH@Z 0x005DB9E0-0x005DBAF5
// TRIED: only one source-form tried; original starts with `sub esp,0xc` for locals before its register pushes (no frame pointer at all - ebp is a plain callee-saved register here), which this form doesn't reproduce. Landing the closest structural translation (mnemonic_similarity 0.712).
// size      277 bytes
// prototype int (__thiscall ?wrap_cent@Buffer@@QAEHPAVFont@@PADHHH@Z)(Buffer* this, Font*, int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DD020 0x006453E0

int Buffer::wrap_cent(Font * a1, char * a2, int a3, int a4, int a5) {
    char *self = reinterpret_cast<char *>(this);

    int savedFont = *reinterpret_cast<int *>(self + 0x52c);
    int savedB = *reinterpret_cast<int *>(self + 0x530);
    int savedC = *reinterpret_cast<int *>(self + 0x534);

    if (a1 != 0 && *reinterpret_cast<int *>(reinterpret_cast<char *>(a1) + 8) != 0) {
        *reinterpret_cast<Font **>(self + 0x52c) = a1;
        *reinterpret_cast<int *>(self + 0x530) = 0;
        *reinterpret_cast<int *>(self + 0x534) = 0;
    }

    int y = a4;
    if (a2 != 0) {
        *g_009b3a9c = 0;
        *reinterpret_cast<int *>(self + 0x18) = 0;
        char *str = a2;
        char *breakPos;
        do {
            breakPos = 0;
            int outParam = a5;
            if (str != 0) {
                *g_009b3a9c = 0;
                unsigned int len = strlen(str);
                breakPos = reinterpret_cast<char *>(find_line_break_l(str, &outParam, len));
                int lineLen;
                if (breakPos == 0) {
                    lineLen = strlen(str);
                }
                else {
                    lineLen = static_cast<int>(breakPos - str);
                }
                write_cent_l(str, a3, y, outParam, lineLen);
            }
            *reinterpret_cast<int *>(self + 0x18) += 1;
            if (*reinterpret_cast<int *>(self + 0x52c) == 0) {
                *reinterpret_cast<int *>(self + 0x52c) = *g_009bb484;
            }
            char *fontPtr = *reinterpret_cast<char **>(self + 0x52c);
            int lineHeight;
            if (*reinterpret_cast<int *>(fontPtr) < 0) {
                lineHeight = *reinterpret_cast<int *>(fontPtr + 0xc);
            }
            else {
                lineHeight = *reinterpret_cast<int *>(fontPtr + 0x10) + *reinterpret_cast<int *>(fontPtr);
            }
            y += lineHeight;
            str = breakPos;
        } while (breakPos != 0);
    }

    *reinterpret_cast<int *>(self + 0x52c) = savedFont;
    *reinterpret_cast<int *>(self + 0x530) = savedB;
    *reinterpret_cast<int *>(self + 0x534) = savedC;
    return y;
}
