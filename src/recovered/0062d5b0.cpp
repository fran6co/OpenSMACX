// ORIGINAL: 0x0062D5B0 BYTE_EXACT
// 0x0062D5B0  ?do_net@@YAXXZ  ->  ?do_net@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class Net { public:
    void process_voice();
};

void __cdecl do_net() {
    if (*g_009be608 && *g_009be600) {
        reinterpret_cast<Net *>(*g_009be608)->process_voice();
    }
}
