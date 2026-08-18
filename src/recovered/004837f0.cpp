// ORIGINAL: 0x004837F0 ?UNK4@NetWin@@QAEXXZ 0x004837F0-0x00483813 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?UNK4@NetWin@@QAEXXZ)(NetWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00608E10 0x0060E740
// 0x004837F0  ?UNK4@NetWin@@QAEXXZ  ->  ?UNK4@NetWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void NetWin::UNK4() {
    char *self = reinterpret_cast<char *>(this);
    CheckBox *checkbox = reinterpret_cast<CheckBox *>(self + 0x1C);
    checkbox->~CheckBox();
    reinterpret_cast<Dialog *>(self + 0xA34)->~Dialog();
    reinterpret_cast<GraphicWin *>(checkbox)->~GraphicWin();
}
