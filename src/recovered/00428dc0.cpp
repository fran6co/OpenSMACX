// ORIGINAL: 0x00428DC0 ?timer_callback_daemon@Credits@@QAEXXZ 0x00428DC0-0x00428DCB BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?timer_callback_daemon@Credits@@QAEXXZ)(Credits* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00428DC0  ?timer_callback_daemon@Credits@@QAEXXZ  ->  ?timer_callback_daemon@Credits@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Advance the credits frame counter and redraw.
Original Offset: 00428DC0
Return Value: n/a
Status: Complete
*/
// `draw_credits` is reached by a tail `jmp`, and the callgraph edge list
// records only `call`, so the scaffolding's own `Credits` shell does not
// declare it. A local shim of the same calling shape is enough: the callee's
// displacement is a relocation and is masked out of the comparison.
class CreditsDrawShim { public: void draw_credits(); };

void Credits::timer_callback_daemon() {
    reinterpret_cast<int *>(this)[0x285] += 1;
    reinterpret_cast<CreditsDrawShim *>(this)->draw_credits();
}
