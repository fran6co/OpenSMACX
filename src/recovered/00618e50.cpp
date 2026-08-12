// ORIGINAL: 0x00618E50 BYTE_EXACT
// 0x00618E50  ?vx_write@Caviar@@SAXHPAXJ@Z  ->  ?vx_write@Caviar@@SAXHPAXJ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct Caviar;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl __write(int, void *, int);

void __cdecl fn_00618e50(int a2, void* a3, int a4) {
    __write(a2, a3, a4);
}
