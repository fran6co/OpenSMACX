// ORIGINAL: 0x00584D30
// 0x00584D30  sub_584d30  ->  _sub_584d30
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
char *__cdecl strncpy(char *dest, const char *src, unsigned int n);

char *__cdecl sub_584d30(char *dest, char *src, unsigned int n) {
    char *result = strncpy(dest, src, n);
    dest[n - 1] = 0;
    return result;
}
