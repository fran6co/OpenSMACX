// ORIGINAL: 0x006112E0 BYTE_EXACT
// name      ?on_left_double_click@SpriteBox@@QAEXHH@Z
// size      80 bytes
// spans     0x006112E0-0x00611330
// prototype void (__thiscall ?on_left_double_click@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00
// 0x006112E0  ?on_left_double_click@SpriteBox@@QAEXHH@Z  ->  ?on_left_double_click@SpriteBox@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__cdecl *fn_click)(int);
typedef int (__stdcall *fn_post)(int, unsigned int, unsigned int, unsigned int);

void SpriteBox::on_left_double_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    Spot *spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(a1, a2, &a2, 0) != -1) {
        fn_click click = *reinterpret_cast<fn_click *>(self - 0xc);
        if (click != 0 && click(a2) != 0) {
            fn_post post = reinterpret_cast<fn_post>(*g_00669314);
            post(*g_009b7b28, 0x100, 0xd, 0);
        }
    }
}
