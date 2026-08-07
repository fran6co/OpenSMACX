// 0x0040B120  ?UNK2@BaseWin@@QAEXHHHH@Z  ->  ?UNK2@BaseWin@@QAEXHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseWin::UNK2(int a1, int a2, int a3, int a4) {
    garrison_click(a1, a2, a3, a4);
}
