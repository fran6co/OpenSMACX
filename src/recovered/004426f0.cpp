// ORIGINAL: 0x004426F0 BYTE_EXACT
// name      ?check_done@DiploWin@@QAEXXZ
// size      70 bytes
// spans     0x004426F0-0x00442736
// prototype void (__thiscall ?check_done@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00442740 0x00592EE0
// 0x004426F0  ?check_done@DiploWin@@QAEXXZ  ->  ?check_done@DiploWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DiploWin::check_done() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0xa24) == 1 && *reinterpret_cast<int *>(self + 0xa28) == 1) {
        message_data(0x150a, *reinterpret_cast<int *>(self + 0xab8), 0, 0, 0, 0);
        if (*g_0093a95c != 0) {
            do_it();
        }
    }
}
