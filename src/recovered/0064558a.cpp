// ORIGINAL: 0x0064558A
// 0x0064558A  ??2@YAPAXI@Z  ->  ??2@YAPAXI@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void *__cdecl __nh_malloc(unsigned int, int);

void *__cdecl fn_0064558a(unsigned int size) {
    return __nh_malloc(size, 1);
}
