// ORIGINAL: 0x00627110 sub_627110 0x00627110-0x00627124 BYTE_EXACT
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00627110  sub_627110  ->  _sub_627110
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Publish a pair of values to two globals and hand the first back.
Original Offset: 00627110
Return Value: the first argument
Status: Complete
*/
extern "C" int __cdecl sub_627110(int a1, int a2) {
    *g_006973dc = a1;
    *g_006973e0 = a2;
    return a1;
}
