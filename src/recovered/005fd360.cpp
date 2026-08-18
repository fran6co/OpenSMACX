// ORIGINAL: 0x005FD360 ?do_all_sounds@@YAXXZ 0x005FD360-0x005FD36B BYTE_EXACT
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x005FD360  ?do_all_sounds@@YAXXZ  ->  ?do_all_sounds@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Clear the pending-sound flag.
Original Offset: 005FD360
Return Value: n/a
Status: Complete
*/
void __cdecl do_all_sounds() {
    *g_009b7b9c = 0;
}
