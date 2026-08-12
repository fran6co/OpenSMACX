// ORIGINAL: 0x004B2210 BYTE_EXACT
// name      ?on_iface_left_click@SocialWin@@QAEHHH@Z
// size      13 bytes
// spans     0x004B2210-0x004B221D
// prototype int (__thiscall ?on_iface_left_click@SocialWin@@QAEHHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004BDDE0
// 0x004B2210  ?on_iface_left_click@SocialWin@@QAEHHH@Z  ->  ?on_iface_left_click@SocialWin@@QAEHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int SocialWin::on_iface_left_click(int a1, int a2) {
    return reinterpret_cast<TutWin *>(g_008c6e68)->reset();
}
