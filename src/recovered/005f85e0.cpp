// ORIGINAL: 0x005F85E0 BYTE_EXACT
// 0x005F85E0  ?on_mousewheel_up@Win@@QAEXH@Z  ->  ?on_mousewheel_up@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Win::on_mousewheel_up(int a1) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);
    char *self = reinterpret_cast<char *>(this);
    Scroll *first = *reinterpret_cast<Scroll **>(self + 0x43c);
    if (first != 0) {
        first->on_mousewheel_up(a1);
    }
    Scroll *second = *reinterpret_cast<Scroll **>(self + 0x440);
    if (second != 0) {
        second->on_mousewheel_up(a1);
    }
}
