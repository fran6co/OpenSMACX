// ORIGINAL: 0x0046A530
// 0x0046A530  ?draw_radius@MapWin@@QAEXHHH@Z  ->  ?draw_radius@MapWin@@QAEXHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MapWin::draw_radius(int a1, int a2, int a3) {
    draw_radius(a1, a2, 1, a3);
}
