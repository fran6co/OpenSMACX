// 0x006104D0  ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z  ->  ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int SpriteBox::init(int a1, int a2, int a3, int a4, Heap * a5) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))
        ->init(a1, a2, a3, a4, a5);
}
