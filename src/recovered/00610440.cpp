// ORIGINAL: 0x00610440 BYTE_EXACT
// 0x00610440  ?init@SpriteBox@@QAEHHHHHH@Z  ->  ?init@SpriteBox@@QAEHHHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int SpriteBox::init(int a1, int a2, int a3, int a4, int a5) {
    char *self = reinterpret_cast<char *>(this);
    close();
    return reinterpret_cast<Dialog *>(
        self + (*reinterpret_cast<int **>(self))[2])->
            init(a1, a2, a3, a4, a5);
}
