// 0x00607A00  ?on_right_click@BaseButton@@QAEXHH@Z  ->  ?on_right_click@BaseButton@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseButton::on_right_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0xa80);
    if (v) {
        timer_callback(0);
    }
}
