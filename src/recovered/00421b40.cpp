// ORIGINAL: 0x00421B40 ?stop_timer@BattleWin@@QAEXXZ 0x00421B40-0x00421B48 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?stop_timer@BattleWin@@QAEXXZ)(BattleWin* this)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00421B40  ?stop_timer@BattleWin@@QAEXXZ  ->  ?stop_timer@BattleWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Stop the battle window's embedded timer at +8.
Original Offset: 00421B40
Return Value: n/a
Status: Complete
*/
// Reached by a tail `jmp`, which the callgraph does not record, so `Time` is
// declared locally with only the member this touches.
class Time { public: void stop(); };

void BattleWin::stop_timer() {
    reinterpret_cast<Time *>(reinterpret_cast<char *>(this) + 8)->stop();
}
