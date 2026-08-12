// ORIGINAL: 0x004406D0 BYTE_EXACT
// 0x004406D0  ?set_modal@DiploPop@@QAEHHP6AHXZPAUWin@@@Z  ->  ?set_modal@DiploPop@@QAEHHP6AHXZPAUWin@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int DiploPop::set_modal(int a1, int (__cdecl *a2)(), Win* a3) {
    return reinterpret_cast<Win *>(this)->set_modal(
        a1 | 1, a2, reinterpret_cast<Win *>(g_007ae820));
}
