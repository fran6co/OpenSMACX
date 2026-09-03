// ORIGINAL: 0x00515400 ?center@Console@@QAEXXZ 0x00515400-0x00515423 BYTE_EXACT
// size      35 bytes
// prototype void (__thiscall ?center@Console@@QAEXXZ)(Console* this)
// callers   5   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046B1F0
// 0x00515400  ?center@Console@@QAEXXZ  ->  ?center@Console@@QAEXXZ
//
// BLOCKED (2026-09-03, P1 calibration) on a mapwin.h declaration this TU does
// not own: 0x0046B1F0 is ?set_center@MapWin@@QAEXHHH@Z, undeclared. It needs
// `void set_center(int, int, int);` on MapWin and a pending_bodies forwarder
// (the body itself is src/recovered/units/0046b1f0.cpp, unclaimed, 275
// bytes). Everything the body reads is already bound here: CursorLastIndex /
// CursorLastX / CursorLastY in console.h and the accessor console_map_win()
// for the single absolute load of 0x007D3C3C the image makes at 0x00515416.
//
// The de-punned body, ready to drop in once set_center is declared:
//
// void Console::center() {
//     const int index = CursorLastIndex;
//     console_map_win()->set_center(CursorLastX[index], CursorLastY[index], 1);
// }

void Console::center() {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);
    int index = *g_009392b8;
    MapWin *map = *reinterpret_cast<MapWin **>(g_007d3c3c);
    map->set_center(g_009392c0[index], g_00939340[index], 1);
}
