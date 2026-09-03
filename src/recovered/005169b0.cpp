// ORIGINAL: 0x005169B0 ?base_me@Console@@QAEXXZ 0x005169B0-0x005169E6 BYTE_EXACT
// size      54 bytes
// prototype void (__thiscall ?base_me@Console@@QAEXXZ)(Console* this)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0041A920 0x004E3C60
// 0x005169B0  ?base_me@Console@@QAEXXZ  ->  ?base_me@Console@@QAEXXZ
//
// BLOCKED (2026-09-03, P1 calibration) on a basewin.h declaration this TU does
// not own: 0x0041A920 is BaseWin::zoom, undeclared. It needs a declaration on
// BaseWin and a pending_bodies forwarder. Everything else is already bound:
// base_find (base.h:533, the three-int overload), LocalFaction (game.h:865,
// 0x00939284 - the scaffold's g_00939284), and tutwin.h's
// TutWinBaseWindow for 0x006A7628.
//
// The de-punned body, ready to drop in once zoom is declared:
//
// void Console::base_me() {
//     const int index = CursorLastIndex;
//     const int result = base_find(CursorLastX[index], CursorLastY[index],
//                                  LocalFaction);
//     if (result >= 0) {
//         TutWinBaseWindow.zoom(result, 0);
//     }
// }

void Console::base_me() {
    int idx = *g_009392b8;
    int faction = *g_00939284;
    int y = g_00939340[idx];
    int x = g_009392c0[idx];
    int result = base_find(x, y, faction);
    if (result >= 0) {
        reinterpret_cast<BaseWin *>(g_006a7628)->zoom(result, 0);
    }
}
