// ORIGINAL: 0x004B1B30 BYTE_EXACT
// 0x004B1B30  ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z  ->  ?bubble_me_elmo2@SocialWin@@QAEXPADHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void SocialWin::bubble_me_elmo2(char * a1, int a2, int a3) {
    reinterpret_cast<Win *>(this)->client_to_screen(&a2, &a3);
    RECT r;
    r.left = a2;
    r.top = a3;
    r.right = a2;
    r.bottom = a3;
    reinterpret_cast<Win *>(this)->set_bubble_text(a1, &r);
}
