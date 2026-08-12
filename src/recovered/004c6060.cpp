// ORIGINAL: 0x004C6060 BYTE_EXACT
// name      sub_4c6060
// size      17 bytes
// spans     0x004C6060-0x004C6071
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// 0x004C6060  sub_4c6060  ->  _sub_4c6060
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Zero the eleven-word table at 0x0090DB24.
Original Offset: 004C6060
Return Value: n/a
Status: Complete
*/
// Returns void, not the catalogued int: eax holds zero at `ret` only because
// it is the `rep stosd` fill value, not as a deliberate result.
extern "C" void __cdecl sub_4c6060() {
    for (int index = 0; index < 0xb; ++index) {
        g_0090db24[index] = 0;
    }
}
