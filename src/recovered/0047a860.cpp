// ORIGINAL: 0x0047A860 BYTE_EXACT
// 0x0047A860  ?on_button_clicked@NetMsg@@QAEXH@Z  ->  ?on_button_clicked@NetMsg@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void NetMsg::on_button_clicked(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x5384)) {
        Time *timer = reinterpret_cast<Time *>(self + 0x538C);
        *reinterpret_cast<int *>(self + 0x5384) = 0;
        timer->close();
        reinterpret_cast<Popup *>(this)->hide();
    }
}
