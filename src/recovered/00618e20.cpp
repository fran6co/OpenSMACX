// ORIGINAL: 0x00618E20 ?vx_free@Caviar@@QAAXPAX@Z 0x00618E20-0x00618E30 BYTE_EXACT
// symbol    ?vx_free@@YAXPAX@Z
// size      16 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// 0x00618E20  ?vx_free@Caviar@@QAAXPAX@Z  ->  ?vx_free@Caviar@@QAAXPAX@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl free(void *p);

void __cdecl vx_free(void *p) {
    if (p) {
        free(p);
    }
}
