// ORIGINAL: 0x00422DE0 BYTE_EXACT
// name      ?on_iface_group_clicked@BattleWin@@QAEXHHH@Z
// size      38 bytes
// spans     0x00422DE0-0x00422E06
// prototype void (__thiscall ?on_iface_group_clicked@BattleWin@@QAEXHHH@Z)(BattleWin* this, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0045CCB0 0x0045D380
// 0x00422DE0  ?on_iface_group_clicked@BattleWin@@QAEXHHH@Z  ->  ?on_iface_group_clicked@BattleWin@@QAEXHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BattleWin::on_iface_group_clicked(int a1, int a2, int a3) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);
    if (a1 == 1) {
        reinterpret_cast<SubInterface *>(this)->release_iface_mode();
        reinterpret_cast<MainInterface *>(g_007af234)->on_iface_group_clicked(1, a2, a3);
    }
}
