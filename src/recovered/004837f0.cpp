// 0x004837F0  ?UNK4@NetWin@@QAEXXZ  ->  ?UNK4@NetWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void NetWin::UNK4() {
    char *self = reinterpret_cast<char *>(this);
    CheckBox *checkbox = reinterpret_cast<CheckBox *>(self + 0x1C);
    checkbox->~CheckBox();
    reinterpret_cast<Dialog *>(self + 0xA34)->~Dialog();
    reinterpret_cast<GraphicWin *>(checkbox)->~GraphicWin();
}
