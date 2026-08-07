// 0x004C4610  ?focus@WorldWin@@QAEXHH@Z  ->  ?focus@WorldWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void WorldWin::focus(int a1, int a2) {
    reinterpret_cast<VCall *>(this)->slot012();
    reinterpret_cast<GraphicWin *>(this)->soft_update();
}
