// ORIGINAL: 0x004E09B0 ?editor_kill_vehicles@@QAEXXZ 0x004E09B0-0x004E09D9 BYTE_EXACT
// size      41 bytes
// prototype void (__thiscall ?editor_kill_vehicles@Console@@QAEXXZ)(Console* this)
// callers   2   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005ABF20 0x005B9510 0x005BFE90
// 0x004E09B0  ?editor_kill_vehicles@Console@@QAEXXZ  ->  ?editor_kill_vehicles@Console@@QAEXXZ
//
// Purpose: editor command - sweep the map clear of vehicles by killing every
//          unit standing on a cursor-marked tile.
//
// PREPARED, NOT HOMED (2026-09-03, P1 calibration). The body below is the
// de-punned member form and compiles as is; the only thing missing is a
// declaration for the free function it calls. `stack_kill` is
// ?stack_kill@@YAXH@Z at 0x005B9510 (src/recovered/units/005b9510.cpp), so it
// needs a pending_bodies forwarder too - the shape is already written there
// beside the veh_ready/world_rocky ones. Its natural header home is veh.h
// beside veh_at.

void Console::editor_kill_vehicles() {
    auto_undo();
    const int index = CursorLastIndex;
    stack_kill(veh_at(CursorLastX[index], CursorLastY[index]));
}
