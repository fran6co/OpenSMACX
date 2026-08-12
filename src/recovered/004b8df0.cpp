// ORIGINAL: 0x004B8DF0 BYTE_EXACT
// name      ?UNK3@StatusWin@@QAEXXZ
// size      65 bytes
// spans     0x004B8DF0-0x004B8E31
// prototype void (__thiscall ?UNK3@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   0   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0041A920 0x00511520 0x005C1D70
// 0x004B8DF0  ?UNK3@StatusWin@@QAEXXZ  ->  ?UNK3@StatusWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void StatusWin::UNK3() {
    int a = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x15bc);
    if (a >= 0) {
        veh_wake(a);
        *g_0093928c = a;
        reinterpret_cast<Console *>(0x9156B0)->set_move(1);
        return;
    }
    int b = *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x15c0);
    if (b >= 0) {
        reinterpret_cast<BaseWin *>(0x6A7628)->zoom(b, 0);
    }
}
