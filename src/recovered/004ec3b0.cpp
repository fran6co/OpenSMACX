// ORIGINAL: 0x004EC3B0 BYTE_EXACT
// 0x004EC3B0  ?base_compute@@YAHH@Z  ->  ?base_compute@@YAHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl base_compute(int a1) {
    if (a1 != 0) {
        *g_00689374 = *g_00689370;
        base_support();
        base_yield();
        base_nutrient();
        base_minerals();
        base_energy();
    } else {
        int g1 = *g_00689370;
        if (*g_00689374 != g1) {
            *g_00689374 = g1;
            base_support();
            base_yield();
            base_nutrient();
            base_minerals();
            base_energy();
        } else {
            return g1;
        }
    }
}
