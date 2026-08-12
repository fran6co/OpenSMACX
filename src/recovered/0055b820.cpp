// ORIGINAL: 0x0055B820 BYTE_EXACT
// name      ?agenda_off@@YAXHHH@Z
// size      80 bytes
// spans     0x0055B820-0x0055B870
// prototype 
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x0055B820  ?agenda_off@@YAXHHH@Z  ->  ?agenda_off@@YAXHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl agenda_off(int a1, int a2, int a3) {
    int mask = ~a3;
    int idx1 = a1 * 0x833 + a2;
    int v = g_0096ca18[idx1];
    v &= mask;
    g_0096ca18[idx1] = v;
    int idx2 = a2 * 0x833 + a1;
    g_0096ca18[idx2] &= mask;
}
