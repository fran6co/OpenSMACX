// CLAIM DROPPED 2026-08-16, and the body is untouched. `Buffer` became
// polymorphic - `virtual ~Buffer()` and `virtual surface_lost()` in place
// of the explicit `LPVOID vtable_` member - which is what the image has
// and what lets a slot-1 call be spelled `surface_lost()`. The scaffold
// cannot lay out a class with virtuals, so it emits `Buffer` opaque here
// and this body stops compiling: `C2065: 'surface_' : undeclared`.
//
// The body is not wrong and was byte-exact the day before. What is gone
// is the ability to CHECK it from a scaffold, so the claim goes with it -
// state is measured, not remembered. It comes back the day this body is
// promoted into the file that owns its class, which is where it belongs.
// ORIGINAL: 0x005D8A30
// name      ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z
// size      238 bytes
// spans     0x005D8A30-0x005D8B1E
// prototype int (__thiscall ?draw_dest@Buffer@@QAEHPAVBuffer@@HHHH@Z)(Buffer* this, Buffer*, int, int, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E5833
// indirect  0x005D8AE7
// WAS-LEVER: same preamble as the other Buffer::draw_* siblings; the callee
// (Sprite::draw_dest) takes `unsigned char *` as its last param where this
// function's own a5 is `int`, so the tail call casts it.

int Buffer::draw_dest(Buffer * a1, int a2, int a3, int a4, int a5) {
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
        *g_009b3a5c = stride_;
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
        int result = reinterpret_cast<Sprite *>(g_009b3a50)->draw_dest(
            a1, a2, a3, a4, reinterpret_cast<unsigned char *>(a5));
        *g_009b3a54 = 0;
        return result;
    }
}
