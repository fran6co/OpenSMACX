// ORIGINAL: 0x005D93C0 BYTE_EXACT
// name      ?draw_multi_table_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z
// size      238 bytes
// spans     0x005D93C0-0x005D94AE
// prototype int (__thiscall ?draw_multi_table_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z)(Buffer* this, Buffer*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E8FF5
// indirect  0x005D9477
// LEVER: same preamble as the other Buffer::draw_* siblings; the callee
// (Sprite::draw_multi_table_dest) takes `unsigned char *` as its last
// param where this function's own a5 is `int`, so the tail call casts it.

int Buffer::draw_multi_table_dest(Buffer * a1, int a2, int a3, int a4, int a5) {
    int transparent = a2;
    if (surface_ == 0) {
        if (a2 == -1) {
            transparent = *reinterpret_cast<unsigned char *>(g_00696d14);
        }
        field_50_ = reinterpret_cast<uint32_t>(ppv_bits_);
        if (field_50_ != 0) {
            field_6C_ = field_6C_ + 1;
        }
        *g_009b3a54 = field_50_;
        *g_009b3a5c = field_4A8_;
        *g_009b3a60 = width_;
        *g_009b3a64 = -(int)height_;
        *g_009b3a68 = width_;
        *g_009b3a6c = -(int)height_;
        *g_009b3a70 = 0;
        *g_009b3a74 = 0;
        *reinterpret_cast<unsigned char *>(g_009b3a58) = (unsigned char)transparent;
        *g_009b3a78 = 0;

        if (surface_ == 0) {
            field_6C_ = field_6C_ - 1;
            if ((int)field_6C_ > 0) {
                goto tail;
            }
        } else {
            field_6C_ = field_6C_ - 1;
            if (field_50_ == 0 || (int)field_6C_ > 0) {
                goto tail;
            }
            {
                typedef int (__stdcall *ReleaseFn)(uint32_t, uint32_t);
                ReleaseFn fn = *reinterpret_cast<ReleaseFn *>(*reinterpret_cast<uint32_t *>(surface_) + 0x80);
                fn(reinterpret_cast<uint32_t>(surface_), field_50_);
            }
        }
        field_50_ = 0;
        field_6C_ = 0;
    }
tail:
    {
        int result = reinterpret_cast<Sprite *>(g_009b3a50)->draw_multi_table_dest(
            a1, a2, a3, a4, reinterpret_cast<unsigned char *>(a5));
        *g_009b3a54 = 0;
        return result;
    }
}
