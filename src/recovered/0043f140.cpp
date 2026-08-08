// 0x0043F140  ?show@DiploPop@@QAEXH@Z  ->  ?show@DiploPop@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DiploPop::show(int a1) {
    char *self = reinterpret_cast<char *>(this);
    Win *win = reinterpret_cast<Win *>(this);
    if (win->is_visible() == 0 &&
        *reinterpret_cast<int *>(self + 0xa94) != -1) {
        win->show(0);
    }
}
