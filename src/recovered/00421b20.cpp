// ORIGINAL: 0x00421B20 ?pulse_timer@BattleWin@@QAEXXZ 0x00421B20-0x00421B37 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?pulse_timer@BattleWin@@QAEXXZ)(BattleWin* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006164D0
// 0x00421B20  ?pulse_timer@BattleWin@@QAEXXZ  ->  ?pulse_timer@BattleWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BattleWin::pulse_timer() {
    reinterpret_cast<Time *>(reinterpret_cast<char *>(this) + 8)->pulse(
        reinterpret_cast<void (__cdecl *)(int)>(g_00422ec0), 0, 0x1388, 5);
}
