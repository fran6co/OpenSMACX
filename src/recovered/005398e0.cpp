// ORIGINAL: 0x005398E0 BYTE_EXACT
// 0x005398E0  ?pop_wait@@YAHXZ  ->  ?pop_wait@@YAHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl pop_wait() {
    if (*g_0093f660 == 0) {
        return 1;
    }
    if (*g_009b2068 != 0) {
        return 0;
    }
    *g_0093fac4 = 1;
    wait_loop();
    *g_0093fac4 = 0;
    return 1;
}
