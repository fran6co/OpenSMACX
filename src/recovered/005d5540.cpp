// ORIGINAL: 0x005D5540
// 0x005D5540  ?resize@GraphicWin@@QAEXHHH@Z  ->  ?resize@GraphicWin@@QAEXHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void GraphicWin::resize(int a1, int a2, int a3) {
    if (a3 == 0) {
        reinterpret_cast<Win *>(this)->nonclient_to_client(&a1, &a2);
    }
    if (buffer_.init(a1, a2, 0, 0) == 0) {
        reinterpret_cast<Win *>(this)->resize(a1, a2, 1);
    }
}
