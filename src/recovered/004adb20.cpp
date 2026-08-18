// ORIGINAL: 0x004ADB20 ?do_menu@SetupWin@@QAEHPAUPopup@@HH@Z 0x004ADB20-0x004ADB66 BYTE_EXACT
// size      70 bytes
// prototype int (__thiscall ?do_menu@SetupWin@@QAEHPAUPopup@@HH@Z)(SetupWin* this, Popup*, int, int)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004ADB70
// 0x004ADB20  ?do_menu@SetupWin@@QAEHPAUPopup@@HH@Z  ->  ?do_menu@SetupWin@@QAEHPAUPopup@@HH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int SetupWin::do_menu(Popup * a1, int a2, int a3) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<Popup **>(self + 0x1014) = a1;
    switch (a2) {
        case 1: {
            int result = do_menu_rightside(reinterpret_cast<char *>(g_008a6268), a3);
            *reinterpret_cast<Popup **>(self + 0x1014) = 0;
            return result;
        }
        default:
            *reinterpret_cast<Popup **>(self + 0x1014) = 0;
            return -1;
    }
}
