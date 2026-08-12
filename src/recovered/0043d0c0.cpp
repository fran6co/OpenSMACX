// ORIGINAL: 0x0043D0C0 BYTE_EXACT
// 0x0043D0C0  ?show@DesignWin@@QAEXH@Z  ->  ?show@DesignWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DesignWin::show(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        ambience(0xD3);
        reinterpret_cast<SubInterface *>(self + 0xA14)->set_iface_mode();
    }
}
