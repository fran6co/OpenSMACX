// 0x004B26B0  ?hide@SocialWin@@QAEXXZ  ->  ?hide@SocialWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void SocialWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        sub_interface_.release_iface_mode();
    }
}
