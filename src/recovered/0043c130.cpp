// ORIGINAL: 0x0043C130 BYTE_EXACT
// name      ?construct_reactor@DesignWin@@QAEXXZ
// size      51 bytes
// spans     0x0043C130-0x0043C163
// prototype void (__thiscall ?construct_reactor@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// 0x0043C130  ?construct_reactor@DesignWin@@QAEXXZ  ->  ?construct_reactor@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DesignWin::construct_reactor() {
    char *self = reinterpret_cast<char *>(this);
    vehdraw_construct_reactor(
        *g_00939284,
        *reinterpret_cast<int *>(self + 0x141fc),
        *reinterpret_cast<int *>(self + 0x14200),
        *reinterpret_cast<int *>(self + 0x1420c),
        *reinterpret_cast<int *>(self + 0x14208),
        reinterpret_cast<Caviar *>(self + 0x1279c));
}
