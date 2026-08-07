// 0x00432040  ?on_selected@Datalink@@QAEXH@Z  ->  ?on_selected@Datalink@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Datalink::on_selected(int a1) {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0x2a38);
    if (v == 0) {
        draw_entry();
    }
}
