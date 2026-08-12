// ORIGINAL: 0x0043C070 BYTE_EXACT
// 0x0043C070  ?construct_chassis@DesignWin@@QAEXXZ  ->  ?construct_chassis@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DesignWin::construct_chassis() {
    char *self = reinterpret_cast<char *>(this);
    vehdraw_construct_chassis(
        *reinterpret_cast<int *>(self + 0x141f4),
        *reinterpret_cast<int *>(self + 0x141fc),
        *reinterpret_cast<int *>(self + 0x14200),
        0,
        *reinterpret_cast<int *>(self + 0x1420c),
        reinterpret_cast<Caviar *>(self + 0x1279c));
}
