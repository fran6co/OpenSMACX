// ORIGINAL: 0x00530600 BYTE_EXACT
// name      ?close_channel@NetDaemon@@QAEXXZ
// size      47 bytes
// spans     0x00530600-0x0053062F
// prototype void (__thiscall ?close_channel@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0
// 0x00530600  ?close_channel@NetDaemon@@QAEXXZ  ->  ?close_channel@NetDaemon@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void NetDaemon::close_channel() {
    if (*g_0093f660 != 0) {
        message_data(0x8241, 0, 0, 0, 0, 0);
        *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x1ba0) = 0;
    }
}
