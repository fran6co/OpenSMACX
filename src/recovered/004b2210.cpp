// ORIGINAL: 0x004B2210 BYTE_EXACT
// 0x004B2210  ?on_iface_left_click@SocialWin@@QAEHHH@Z  ->  ?on_iface_left_click@SocialWin@@QAEHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int SocialWin::on_iface_left_click(int a1, int a2) {
    return reinterpret_cast<TutWin *>(g_008c6e68)->reset();
}
