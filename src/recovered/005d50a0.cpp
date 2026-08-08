// 0x005D50A0  ?init@GraphicWin@@QAEHPAURECT@@PADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z  ->  ?init@GraphicWin@@QAEHPAURECT@@PADHPAUWin@@PAUMenu@@PAUBorderSizing@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int GraphicWin::init(RECT * a1, char * a2, int a3, Win * a4, Menu * a5, BorderSizing * a6) {
    if (a1 == 0) {
        return 0x10;
    }
    init(a1->left, a1->top, a1->right - a1->left, a1->bottom - a1->top, a2, a3, a4, a5, a6);
}
