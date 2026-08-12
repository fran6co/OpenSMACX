// ORIGINAL: 0x00592D90 BYTE_EXACT
// name      sub_592d90
// size      26 bytes
// spans     0x00592D90-0x00592DAA
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x00592D90  sub_592d90  ->  _sub_592d90
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Scale the caller's value by the stored setting, in sixteenths.
Original Offset: 00592D90
Return Value: the scaled value
Status: Complete
*/
// Catalogued as taking no parameters; the body reads one at [ebp+8].
extern "C" int __cdecl sub_592d90(int a1) {
    // A plain signed `/ 16` is what emits the sar/and/add rounding tail.
    return (*g_00945aec + 0x10) * a1 / 16;
}
