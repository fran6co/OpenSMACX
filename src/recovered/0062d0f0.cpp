// ORIGINAL: 0x0062D0F0 sub_62d0f0 0x0062D0F0-0x0062D0F8 BYTE_EXACT
// size      8 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062C8E0
// 0x0062D0F0  sub_62d0f0  ->  _sub_62d0f0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl sub_62c8e0();

int __cdecl sub_62d0f0() {
    sub_62c8e0();
    return 0;
}
