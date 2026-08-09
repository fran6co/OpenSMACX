// ORIGINAL: 0x005BFAE0
// 0x005BFAE0  ?X_pop_ask@@YAHPADPBDHP6AHXZH@Z  ->  ?X_pop_ask@@YAHPADPBDHP6AHXZH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
// The 6-param overload: same as the subject but with an extra int8_t*
// inserted in 4th position (the subject passes NULL for it).
int __cdecl X_pop_ask(int8_t*, const int8_t*, int, int8_t*, int (__cdecl *)(), int);

int __cdecl X_pop_ask(int8_t* a1, const int8_t* a2, int a3, int (__cdecl *a4)(), int a5) {
    return X_pop_ask(a1, a2, a3, 0, a4, a5);
}
