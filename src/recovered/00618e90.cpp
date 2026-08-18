// ORIGINAL: 0x00618E90 ?vx_tell@Caviar@@QAAXH@Z 0x00618E90-0x00618E9E BYTE_EXACT
// size      14 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00650220
// 0x00618E90  ?vx_tell@Caviar@@QAAXH@Z  ->  ?vx_tell@Caviar@@QAAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct Caviar;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl __tell(int);

int __cdecl fn_00618e90(int a1, int a2) {
    return __tell(a1);
}
