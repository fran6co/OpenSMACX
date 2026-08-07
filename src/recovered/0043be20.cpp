// 0x0043BE20  ?select_special_2@DesignWin@@QAEXXZ  ->  ?select_special_2@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DesignWin::select_special_2() {
    select_special(1);
}
