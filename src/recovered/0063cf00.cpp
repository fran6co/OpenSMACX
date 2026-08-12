// ORIGINAL: 0x0063CF00 BYTE_EXACT
// name      deflateInit_
// size      37 bytes
// spans     0x0063CF00-0x0063CF25
// prototype 
// callers   1   call targets   1
// 0x0063CF00  deflateInit_  ->  _deflateInit_
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl sub_63cf30(void *, int, int, int, int, int, const char *, int);

int __cdecl deflateInit_(void *a1, int a2, const char *a3, int a4) {
    return sub_63cf30(a1, a2, 8, 0xf, 8, 0, a3, a4);
}
