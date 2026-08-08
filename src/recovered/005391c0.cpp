// 0x005391C0  ?net_treaty_on@@YAXHHHH@Z  ->  ?net_treaty_on@@YAXHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl net_treaty_on(int a1, int a2, int a3, int a4) {
    if (*g_0093f660 != 0) {
        log_say(reinterpret_cast<char *>(g_0068d444), a1, a2, a3);
        message_data(0x2441, 0, a1, a2, a3, 0);
        if (a4 != 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x441);
        }
    } else {
        treaty_on(a1, a2, a3);
    }
}
