// ORIGINAL: 0x00618E70 BYTE_EXACT
// name      ?vx_seek@Caviar@@QAAXHJH@Z
// size      24 bytes
// spans     0x00618E70-0x00618E88
// prototype 
// callers   0   call targets   1
// 0x00618E70  ?vx_seek@Caviar@@QAAXHJH@Z  ->  ?vx_seek@Caviar@@QAAXHJH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct Caviar;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl __lseek(int, int, int);

void __cdecl fn_00618e70(Caviar* a1, int a2, int a3, int a4) {
    __lseek(reinterpret_cast<int>(a1), a2, a3);
}
