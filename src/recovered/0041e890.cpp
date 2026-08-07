// 0x0041E890  ?show@BaseWin@@QAEXH@Z  ->  ?show@BaseWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void BaseWin::show(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (!reinterpret_cast<Win *>(self)->is_visible()) {
        reinterpret_cast<SubInterface *>(self + 0xa14)->set_iface_mode();
    }
}
