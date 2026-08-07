// 0x0041B4A0  ?on_scrolled@BaseWin@@QAEXHH@Z  ->  ?on_scrolled@BaseWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseWin::on_scrolled(int a1, int a2) {
    switch (a1) {
        case 1:
            *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x40b10) = a2;
            draw_facilities(1);
            break;
    }
}
