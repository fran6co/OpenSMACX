// ORIGINAL: 0x0045CD80 ?on_iface_left_double_click@MainInterface@@QAEXHH@Z 0x0045CD80-0x0045CDB0 BYTE_EXACT
// size      48 bytes
// prototype void (__thiscall ?on_iface_left_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B8990 0x005FA7E0
// 0x0045CD80  ?on_iface_left_double_click@MainInterface@@QAEXHH@Z  ->  ?on_iface_left_double_click@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MainInterface::on_iface_left_double_click(int a1, int a2) {
    if (in_box(a1, a2, reinterpret_cast<RECT *>(g_007af50c))) {
        reinterpret_cast<StatusWin *>(g_008c5568)->
            on_left_double_click(a1, a2);
    }
}
