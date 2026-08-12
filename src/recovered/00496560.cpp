// ORIGINAL: 0x00496560 BYTE_EXACT
// name      ?on_button_clicked@QuayleWin@@QAEXH@Z
// size      81 bytes
// spans     0x00496560-0x004965B1
// prototype void (__thiscall ?on_button_clicked@QuayleWin@@QAEXH@Z)(QuayleWin* this, int)
// callers   0   call targets   2
// 0x00496560  ?on_button_clicked@QuayleWin@@QAEXH@Z  ->  ?on_button_clicked@QuayleWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void QuayleWin::on_button_clicked(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 999) {
        if (*reinterpret_cast<int *>(self + 0x2144) + 1 != 0) {
            *reinterpret_cast<int *>(self + 0x2144) = -1;
            reinterpret_cast<VCall *>(this)->slot062();
            return;
        }
        reinterpret_cast<ListBox *>(self + 0x2158)->close();
        reinterpret_cast<VCall *>(this)->slot058();
        reinterpret_cast<GraphicWin *>(this)->close();
    }
}
