// ORIGINAL: 0x00539510 BYTE_EXACT
// name      ?net_loan@@YAXHHHH@Z
// size      111 bytes
// spans     0x00539510-0x0053957F
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0
// 0x00539510  ?net_loan@@YAXHHHH@Z  ->  ?net_loan@@YAXHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Indexed table bases: the address itself is added to a computed index, so
// the const-pointer spelling (which /O2 folds into a literal and loses the
// add) is wrong here - see "the lever that keeps working" above.
extern int g_0096caa4[];
extern int g_0096cac4[];

void __cdecl net_loan(int a1, int a2, int a3, int a4) {
    if (*g_0093f660 != 0) {
        log_say(reinterpret_cast<char *>(g_0068d4cc), a1, a2, a3);
        message_data(0x244e, 0, a1, a2, a3, a4);
    } else {
        int offset = (a1 * 0x833 + a2) * 4;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096caa4) + offset) = a3;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096cac4) + offset) = a4;
    }
}
