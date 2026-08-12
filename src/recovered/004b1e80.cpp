// ORIGINAL: 0x004B1E80 BYTE_EXACT
// name      ?on_key_click@SocialWin@@QAEHHH@Z
// size      53 bytes
// spans     0x004B1E80-0x004B1EB5
// prototype int (__thiscall ?on_key_click@SocialWin@@QAEHHH@Z)(SocialWin* this, int, int)
// callers   0   call targets   3
// 0x004B1E80  ?on_key_click@SocialWin@@QAEHHH@Z  ->  ?on_key_click@SocialWin@@QAEHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int SocialWin::on_key_click(int a1, int a2) {
    reinterpret_cast<TutWin *>(0x008C6E68)->reset();
    switch (a2) {
        case 27:
            reset();
            // fallthrough
        case 13:
            exit();
            break;
    }
    return 1;
}
