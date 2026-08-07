// 0x00477EE0  ?on_mouse_leave@MultiWin@@QAEXHH@Z  ->  ?on_mouse_leave@MultiWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MultiWin::on_mouse_leave(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x1168) >= 0) {
        *reinterpret_cast<int *>(self + 0x1168) = -1;
        draw(0);
    }
}
