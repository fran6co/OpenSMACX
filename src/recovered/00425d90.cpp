// 0x00425D90  ?draw_all_leaders@CouncWin@@QAEXXZ  ->  ?draw_all_leaders@CouncWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void CouncWin::draw_all_leaders() {
    for (int i = 1; i < 8; i++) {
        draw_leader(i);
    }
}
