// ORIGINAL: 0x0062D5D0 ?check_net@@YAXXZ 0x0062D5D0-0x0062D5E9 BYTE_EXACT
// size      25 bytes
// prototype 
// callers   21   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0062D5D0  ?check_net@@YAXXZ  ->  ?check_net@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class Net { public:
    void check_polling();
};

void __cdecl check_net() {
    Net *net = reinterpret_cast<Net *>(*g_009be608);
    if (net == 0) {
        return;
    }
    if (*g_009be600 == 0) {
        return;
    }
    net->check_polling();
}
