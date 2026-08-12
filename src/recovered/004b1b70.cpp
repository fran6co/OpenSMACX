// ORIGINAL: 0x004B1B70 BYTE_EXACT
// name      ?bubble_me_elmo2@SocialWin@@QAGXPADHH@Z
// size      66 bytes
// spans     0x004B1B70-0x004B1BB2
// prototype void (__stdcall ?bubble_me_elmo2@SocialWin@@QAGXPADHH@Z)(SocialWin* this, int8*, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005ED240 0x005F83D0
// 0x004B1B70  ?bubble_me_elmo2@SocialWin@@QAGXPADHH@Z  ->  ?bubble_me_elmo2@SocialWin@@QAGXPADHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __stdcall bubble_me_elmo2(char *a1, int a2, int a3) {
    reinterpret_cast<Win *>(g_007ae820)->client_to_screen(&a2, &a3);
    RECT rect;
    rect.left = a2;
    rect.top = a3;
    rect.right = a2;
    rect.bottom = a3;
    reinterpret_cast<Win *>(g_007ae820)->set_bubble_text(a1, &rect);
}
