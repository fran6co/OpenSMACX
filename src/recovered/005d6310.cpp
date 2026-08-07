// 0x005D6310  ?on_mouse_move@GraphicWin@@QAEXHHIH@Z  ->  ?on_mouse_move@GraphicWin@@QAEXHHIH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void GraphicWin::on_mouse_move(int a1, int a2, unsigned int a3, int a4) {
    reinterpret_cast<Win *>(this)->on_mouse_move(a1, a2, a3, a4);
}
