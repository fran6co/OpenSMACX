// ORIGINAL: 0x00515400 BYTE_EXACT
// name      ?center@Console@@QAEXXZ
// size      35 bytes
// spans     0x00515400-0x00515423
// prototype void (__thiscall ?center@Console@@QAEXXZ)(Console* this)
// callers   5   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046B1F0
// 0x00515400  ?center@Console@@QAEXXZ  ->  ?center@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

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
