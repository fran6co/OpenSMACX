// ORIGINAL: 0x0045D110
// 0x0045D110  ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z  ->  ?on_iface_mouse_leave@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MainInterface::on_iface_mouse_leave(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<StatusWin *>(g_008c5568)->reset_active();
    if (*reinterpret_cast<int *>(self + 0x20cd8) == 1) {
        *reinterpret_cast<int *>(self + 0x20cd8) = 0;
        reinterpret_cast<MainInterface *>(self - 0xa14)->
            redraw_complete();
    }
}
