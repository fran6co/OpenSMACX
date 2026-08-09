// ORIGINAL: 0x00611240
// 0x00611240  ?on_right_down@SpriteBox@@QAEXHH@Z  ->  ?on_right_down@SpriteBox@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int(__cdecl *SpriteFn)(int);
typedef int(__stdcall *SendFn)(int, int, int, int);

void SpriteBox::on_right_down(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    Spot *spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(a1, a2, &a2, 0) == -1) {
        return;
    }
    SpriteFn fn = *reinterpret_cast<SpriteFn *>(self - 0x14);
    if (!fn) {
        return;
    }
    if (fn(a2) == 0) {
        return;
    }
    (*reinterpret_cast<SendFn *>(g_00669314))(*g_009b7b28, 0x100, 0xd, 0);
}
