// 0x00427130  ?show@CouncWin@@QAEHH@Z  ->  ?show@CouncWin@@QAEHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int CouncWin::show(int a1) {
    int visible = reinterpret_cast<Win *>(this)->is_visible();
    if (visible) {
        return visible;
    }
    sub_interface_.set_iface_mode();
}
