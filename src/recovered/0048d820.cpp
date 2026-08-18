// ORIGINAL: 0x0048D820 sub_48d820 0x0048D820-0x0048D83E BYTE_EXACT
// size      30 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0048D820  sub_48d820  ->  _sub_48d820
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Recompute the derived total from its three inputs.
Original Offset: 0048D820
Return Value: n/a
Status: Complete
*/
// Returns void, not the catalogued int: nothing establishes eax before `ret`,
// and forcing `int` with `return 0` adds a trailing `xor eax,eax` that grows
// the body from 30 bytes to 32.
extern "C" void __cdecl sub_48d820() {
    *g_008578b8 = (*g_008577e4 - *g_00686b74 * 3) - *g_00686b8c;
}
