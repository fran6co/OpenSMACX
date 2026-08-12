// ORIGINAL: 0x00459260 BYTE_EXACT
// 0x00459260  ?timer_daemon@InfoWin@@QAAXH@Z  ->  ?timer_daemon@InfoWin@@QAAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl InfoWin::timer_daemon(int a1) {
    if (!*g_0068f21c) {
        reinterpret_cast<InfoWin *>(g_007ad2a0)->timer_proc(reinterpret_cast<int>(this));
    }
}
