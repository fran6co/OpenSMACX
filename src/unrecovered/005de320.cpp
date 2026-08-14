// ORIGINAL: 0x005DE320
// RULED-OUT: MISMATCH #2 push/sub - the disassembly reads no stack args and
//            takes the receiver in ecx (no [ecx+N] though; it is only used
//            to call Buffer methods), so this is modelled as a Buffer
//            __thiscall member instead of the placeholder's free __stdcall
//            function; stack-frame shape still differs after that.
// name      sub_5de320
// size      421 bytes
// spans     0x005DE320-0x005DE4C5
// prototype int (__stdcall sub_5DE320)(LPCSTR lpString)
// callers   1   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DC410 0x005DC7C0 0x005DD020 0x006453E0 0x00645470 0x00645930
// To start: tools/decomp_status.py --work 0x005DE320

class Sub5DE320Target {
 public:
  int run(LPCSTR a1);
};

int Sub5DE320Target::run(LPCSTR a1) {
    Buffer *const buf = reinterpret_cast<Buffer *>(this);

    if (*g_009b3a98 == 0) {
        return 0;
    }
    if (a1 == 0) {
        return *g_009b3a94;
    }
    if (*a1 == 0) {
        return *g_009b3a94;
    }

    const char *text = a1;
    while (true) {
        if (*reinterpret_cast<char *>(g_009b33cc) != 0) {
            strcat(reinterpret_cast<char *>(g_009b33cc), reinterpret_cast<char *>(g_00682820));
            unsigned int len = strlen(reinterpret_cast<char *>(g_00696ce0));
            int width = buf->text_width(reinterpret_cast<char *>(g_00696ce0), static_cast<int>(len));
            *g_009b3a9c = *g_009b3a9c + width;
        }
        const char *next = text;
        if (*g_009b3a9c < *g_009b3a98) {
            unsigned int len = strlen(text);
            int room = *g_009b3a98 - *g_009b3a9c;
            next = reinterpret_cast<char *>(
                buf->find_line_break_l(const_cast<char *>(text), &room, static_cast<int>(len)));
        }
        if (next == 0) {
            break;
        }
        unsigned int prefix_len = strlen(reinterpret_cast<char *>(g_009b33cc));
        memcpy(reinterpret_cast<char *>(g_009b33cc) + prefix_len, text,
               static_cast<unsigned int>(next - text));
        (reinterpret_cast<char *>(g_009b33cc) + prefix_len)[next - text] = 0;

        int w = *g_009b3a98;
        int h = *g_009b3a94;
        int v = *g_009b3a90;
        unsigned int total_len = strlen(reinterpret_cast<char *>(g_009b33cc));
        buf->write_cent_l(reinterpret_cast<char *>(g_009b33cc), v, h, w, static_cast<int>(total_len));

        *reinterpret_cast<char *>(g_009b33cc) = 0;
        *g_009b3a9c = 0;

        buf->field_18_ = buf->field_18_ + 1;
        *g_009b3a94 = *g_009b3a94 + buf->text_line_height();

        text = next;
    }

    strcat(reinterpret_cast<char *>(g_009b33cc), const_cast<char *>(text));
    return *g_009b3a94;
}
