// ORIGINAL: 0x004B8990
// 0x004B8990  ?on_left_double_click@StatusWin@@QAEXHH@Z  ->  ?on_left_double_click@StatusWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void StatusWin::on_left_double_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0x15D8) = 1;
    on_left_click(a1, a2);
    *reinterpret_cast<int *>(self + 0x15D8) = 0;
}
