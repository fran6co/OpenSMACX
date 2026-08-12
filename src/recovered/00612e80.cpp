// ORIGINAL: 0x00612E80 BYTE_EXACT
// 0x00612E80  ?on_left_click@Dialogs@@QAEXHH@Z  ->  ?on_left_click@Dialogs@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Dialogs::on_left_click(int a1, int a2) {
    char *const self = reinterpret_cast<char *>(this);
    switch (*reinterpret_cast<int *>(self - 8)) {
    case 1: case 2: case 3: case 4: case 5: case 6: case 7:
    case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16:
        reinterpret_cast<SpriteBox *>(self - 0x8C)->on_left_click(a1, a2);
        break;
    }
}
