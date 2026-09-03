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
// ORIGINAL: 0x005D8DF0 ?draw_dest_source@Buffer@@QAEHPAVBuffer@@HHHPAE@Z 0x005D8DF0-0x005D8EDE
// size      238 bytes
// prototype int (__thiscall ?draw_dest_source@Buffer@@QAEHPAVBuffer@@HHHPAE@Z)(Buffer* this, Buffer*, int, int, int, unsigned int8*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E741C
// indirect  0x005D8EA7
// WAS-LEVER: same preamble as the other Buffer::draw_* siblings; a5 is already
// `unsigned char *` in both this function's own signature and the callee's,
// so the tail call forwards it with no cast.

int Buffer::draw_dest_source(Buffer * a1, int a2, int a3, int a4, unsigned char * a5) {
    int transparent = a2;
    if (surface_ == 0) {
        if (a2 == -1) {
            transparent = *reinterpret_cast<unsigned char *>(WinFillColour);
        }
        locked_bits_ = reinterpret_cast<uint32_t>(ppv_bits_);
        if (locked_bits_ != 0) {
            surface_lock_count_ = surface_lock_count_ + 1;
        }
        BlitSourceBits = locked_bits_;
        BlitSourceField4A8 = stride_;
        BlitSourceWidth = width_;
        BlitSourceNegHeight = -(int)height_;
        BlitClipWidth = width_;
        BlitClipNegHeight = -(int)height_;
        BlitSourceField70 = 0;
        BlitSourceField74 = 0;
        *reinterpret_cast<unsigned char *>(BlitTransparentIndex) = (unsigned char)transparent;
        BlitSourceField78 = 0;

        if (surface_ == 0) {
            surface_lock_count_ = surface_lock_count_ - 1;
            if ((int)surface_lock_count_ > 0) {
                goto tail;
            }
        } else {
            surface_lock_count_ = surface_lock_count_ - 1;
            if (locked_bits_ == 0 || (int)surface_lock_count_ > 0) {
                goto tail;
            }
            {
                typedef int (__stdcall *ReleaseFn)(uint32_t, uint32_t);
                ReleaseFn fn = *reinterpret_cast<ReleaseFn *>(*reinterpret_cast<uint32_t *>(surface_) + 0x80);
                fn(reinterpret_cast<uint32_t>(surface_), locked_bits_);
            }
        }
        locked_bits_ = 0;
        surface_lock_count_ = 0;
    }
tail:
    {
        int result = reinterpret_cast<Sprite *>(g_009b3a50)->draw_dest_source(
            a1, a2, a3, a4, a5);
        BlitSourceBits = 0;
        return result;
    }
}
