// ORIGINAL: 0x0043F200 ?timer_callback_daemon@DiploPop@@QAAXHH@Z 0x0043F200-0x0043F245 BYTE_EXACT
// size      69 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D5930 0x0062A070
// 0x0043F200  ?timer_callback_daemon@DiploPop@@QAAXHH@Z  ->  ?timer_callback_daemon@DiploPop@@QAAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Free function, not a DiploPop instance method: the disassembly reads only
// [ebp+8] (unused) and [ebp+0xc] (the object pointer), with no [ecx+N]
// access anywhere - two plain stack ints, no hidden `this`. Defining this as
// a non-static `DiploPop::` member under `__cdecl` still gets an implicit
// `this` pushed ahead of the declared parameters (MSVC's ABI for `__cdecl`
// on a non-static member), which shifted the frame offset from 0xc to 0x10
// against the original - so the scaffolding's declared member is left
// unused and this is a plain function instead.
void __cdecl timer_callback_daemon(int a1, int a2) {
    if (a2) {
        char *self = reinterpret_cast<char *>(a2);
        Flic *flic = reinterpret_cast<Flic *>(self + 0x15cc);
        char *flic_bytes = reinterpret_cast<char *>(flic);
        ++*reinterpret_cast<uint32_t *>(self + 0x20b0);
        flic->decode_frame(*reinterpret_cast<int *>(flic_bytes + 0x5ac),
                            *reinterpret_cast<int *>(flic_bytes + 0x5b0));
        reinterpret_cast<GraphicWin *>(g_007ae820)->soft_update(
            reinterpret_cast<RECT *>(self + 0x1b58));
    }
}
