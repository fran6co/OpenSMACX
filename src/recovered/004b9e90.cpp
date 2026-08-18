// ORIGINAL: 0x004B9E90 ?UNK4@StatusWin@@QAEXXZ 0x004B9E90-0x004B9E9D BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ?UNK4@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// 0x004B9E90  ?UNK4@StatusWin@@QAEXXZ  ->  ?UNK4@StatusWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Raise bit 5 of the status-window redraw flags.
Original Offset: 004B9E90
Return Value: n/a
Status: Complete
*/
void StatusWin::UNK4() {
    *g_00939ed8 |= 0x20;
}
