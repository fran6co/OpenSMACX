// ORIGINAL: 0x0051F2C0 ?wait_2@@YAXXZ 0x0051F2C0-0x0051F2D8 BYTE_EXACT
// size      24 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0051F2C0  ?wait_2@@YAXXZ  ->  ?wait_2@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class NetDaemon { public:
    void net_tasks();
};
void __cdecl wait_task();

void __cdecl wait_2() {
    if (*g_0093f660) {
        reinterpret_cast<NetDaemon *>(g_0093cd90)->net_tasks();
    } else {
        wait_task();
    }
}
