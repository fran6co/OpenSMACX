// ORIGINAL: 0x005169B0 BYTE_EXACT
// name      ?base_me@Console@@QAEXXZ
// size      54 bytes
// spans     0x005169B0-0x005169E6
// prototype void (__thiscall ?base_me@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// 0x005169B0  ?base_me@Console@@QAEXXZ  ->  ?base_me@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::base_me() {
    int idx = *g_009392b8;
    int faction = *g_00939284;
    int y = g_00939340[idx];
    int x = g_009392c0[idx];
    int result = base_find(x, y, faction);
    if (result >= 0) {
        reinterpret_cast<BaseWin *>(g_006a7628)->zoom(result, 0);
    }
}
