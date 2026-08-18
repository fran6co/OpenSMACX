// ORIGINAL: 0x00593480 sub_593480 0x00593480-0x005934A2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
// 0x00593480  sub_593480  ->  _sub_593480
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_593480(int a1) {
    reinterpret_cast<NetDaemon *>(g_0093cd90)->synch(0x17, a1, 0, 0, 0, 1, 0x2101);
}
