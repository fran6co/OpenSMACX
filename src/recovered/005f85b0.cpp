// ORIGINAL: 0x005F85B0
// 0x005F85B0  ?on_mousewheel_down@Win@@QAEXH@Z  ->  ?on_mousewheel_down@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Win::on_mousewheel_down(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Scroll *scroll = *reinterpret_cast<Scroll **>(self + 0x43C);
    if (scroll) {
        scroll->on_mousewheel_down(a1);
    }
    scroll = *reinterpret_cast<Scroll **>(self + 0x440);
    if (scroll) {
        scroll->on_mousewheel_down(a1);
    }
}
