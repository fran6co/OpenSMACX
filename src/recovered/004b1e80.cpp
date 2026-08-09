// ORIGINAL: 0x004B1E80
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
