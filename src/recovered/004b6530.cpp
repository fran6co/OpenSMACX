// ORIGINAL: 0x004B6530 ?reset_active@StatusWin@@QAEXXZ 0x004B6530-0x004B6569 BYTE_EXACT
// size      57 bytes
// prototype void (__thiscall ?reset_active@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004B4CF0
// 0x004B6530  ?reset_active@StatusWin@@QAEXXZ  ->  ?reset_active@StatusWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void StatusWin::reset_active() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x15c4) >= 0 ||
        *reinterpret_cast<int *>(self + 0x15c8) >= 0) {
        int arg1 = *reinterpret_cast<int *>(self + 0x15cc);
        *reinterpret_cast<int *>(self + 0x15c8) = -1;
        *reinterpret_cast<int *>(self + 0x15c4) = -1;
        draw_active(arg1, *reinterpret_cast<int *>(self + 0x15d0), 0);
    }
}
