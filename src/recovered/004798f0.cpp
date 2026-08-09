// ORIGINAL: 0x004798F0
// 0x004798F0  ?on_redraw@MultiWin@@QAEXXZ  ->  ?on_redraw@MultiWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MultiWin::on_redraw() {
    draw(0);
}
