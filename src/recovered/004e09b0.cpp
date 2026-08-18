// ORIGINAL: 0x004E09B0 ?editor_kill_vehicles@Console@@QAEXXZ 0x004E09B0-0x004E09D9 BYTE_EXACT
// size      41 bytes
// prototype void (__thiscall ?editor_kill_vehicles@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005ABF20 0x005B9510 0x005BFE90
// 0x004E09B0  ?editor_kill_vehicles@Console@@QAEXXZ  ->  ?editor_kill_vehicles@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::editor_kill_vehicles() {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);
    auto_undo();
    int index = *g_009392b8;
    stack_kill(veh_at(g_009392c0[index], g_00939340[index]));
}
