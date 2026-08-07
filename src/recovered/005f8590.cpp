// 0x005F8590  ?on_mousewheel_up_horz@Win@@QAEXH@Z  ->  ?on_mousewheel_up_horz@Win@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Win::on_mousewheel_up_horz(int a1) {
    Scroll *scroll = *reinterpret_cast<Scroll **>(reinterpret_cast<char *>(this) + 0x440);
    if (scroll) {
        scroll->on_mousewheel_up(a1);
    }
}
