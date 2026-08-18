// ORIGINAL: 0x00500120 ?clear@@YAXXZ 0x00500120-0x00500128 BYTE_EXACT
// size      8 bytes
// prototype 
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00500120  ?clear@@YAXXZ  ->  ?clear@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Clear the first byte of the shared caption buffer.
Original Offset: 00500120
Return Value: n/a
Status: Complete
*/
void __cdecl clear() {
    *reinterpret_cast<char *>(g_009b86a0) = 0;
}
