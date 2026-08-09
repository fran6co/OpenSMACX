// ORIGINAL: 0x00420A40
// 0x00420A40  ?on_nc_hittest@BaseWin@@QAEHHH@Z  ->  ?on_nc_hittest@BaseWin@@QAEHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int BaseWin::on_nc_hittest(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 >= 0x17 &&
        a1 <= *reinterpret_cast<int *>(self + 0x4c4) - 0x37) {
        return reinterpret_cast<Win *>(this)->on_nc_hittest(a1, a2);
    }
    return 2;
}
