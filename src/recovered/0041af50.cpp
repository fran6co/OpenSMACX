// ORIGINAL: 0x0041AF50 BYTE_EXACT
// 0x0041AF50  ?on_key_click@BaseWin@@QAEXHH@Z  ->  ?on_key_click@BaseWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseWin::on_key_click(int a1, int a2) {
    reinterpret_cast<Console *>(g_00939444)->on_key_click(a1, a2);
}
