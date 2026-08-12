// ORIGINAL: 0x0041AF50 BYTE_EXACT
// name      ?on_key_click@BaseWin@@QAEXHH@Z
// size      25 bytes
// spans     0x0041AF50-0x0041AF69
// prototype void (__thiscall ?on_key_click@BaseWin@@QAEXHH@Z)(BaseWin* this, int, int)
// callers   0   call targets   1
// 0x0041AF50  ?on_key_click@BaseWin@@QAEXHH@Z  ->  ?on_key_click@BaseWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseWin::on_key_click(int a1, int a2) {
    reinterpret_cast<Console *>(g_00939444)->on_key_click(a1, a2);
}
