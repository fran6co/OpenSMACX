// 0x004ADAF0  ?do_menu@SetupWin@@QAEHPADHH@Z  ->  ?do_menu@SetupWin@@QAEHPADHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int SetupWin::do_menu(int8* a1, int a2, int a3) {
    switch (a2) {
        case 1:
            return do_menu_rightside(a1, a3);
        default:
            return -1;
    }
}
