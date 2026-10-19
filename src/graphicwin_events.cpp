#include "stdafx.h"
#include "graphicwin.h"

// The original callback calls update rather than expanding its implementation.
// Keep that implementation outside this event TU; this is a current source
// boundary, not an assertion about the original filename.
// Shared surface callback: GraphicWin constructor5D4D2B installs
// table66FC50 slot63 ->404290; AlphaMovie/MapWin/PlanWin/Console inherit it.
// The catalogue named AlphaMovie, but this body uses the GraphicWin receiver.
/*
// ORIGINAL: 0x00404290 ?update@AlphaMovie@@QAEXXZ 0x00404290-0x00404298 BYTE_EXACT
// symbol    ?on_update@GraphicWin@@UAEXXZ
// size      8 bytes
// prototype void (__thiscall ?update@AlphaMovie@@QAEXXZ)(AlphaMovie* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D56B0
Status: Complete
*/
void GraphicWin::on_update() {
    update(0);
}
