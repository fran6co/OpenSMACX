// ORIGINAL: 0x0043D0F0 BYTE_EXACT
// 0x0043D0F0  ?hide@DesignWin@@QAEXXZ  ->  ?hide@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void DesignWin::hide() {
    char *self = reinterpret_cast<char *>(this);
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(self + 0xa14)->release_iface_mode();
        ambience(0xd4);
    }
}
