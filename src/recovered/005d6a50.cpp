// ORIGINAL: 0x005D6A50
// 0x005D6A50  ?set_def_nc_sprites@GraphicWin@@QAAXPAUNCSprites@@@Z  ->  ?set_def_nc_sprites@GraphicWin@@QAAXPAUNCSprites@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// The disassembly reads a SINGLE stack parameter at [esp+4] and never
// touches [esp+8] or ecx - there is no `this` here. Declaring the
// scaffold's GraphicWin::set_def_nc_sprites member out-of-line would make
// it a non-static __cdecl member instead, which pushes an implicit `this`
// ahead of a1 and shifts a1 to [esp+8] (measured: SHAPE_EXACT with the
// very first `mov` reading 0x8 instead of 0x4). So this is defined as the
// genuinely free function the bytes show, leaving the class's declared
// member undefined (never odr-used, so that is fine).
void __cdecl set_def_nc_sprites(NCSprites *a1) {
    if (a1 != 0) {
        char *src = reinterpret_cast<char *>(a1);
        *g_009b3394 = *reinterpret_cast<int *>(src + 0x0);
        *g_009b339c = *reinterpret_cast<int *>(src + 0x4);
        *g_009b3398 = *reinterpret_cast<int *>(src + 0x8);
        *g_009b33a0 = *reinterpret_cast<int *>(src + 0xc);
        *g_009b33a4 = *reinterpret_cast<int *>(src + 0x10);
        *g_009b33ac = *reinterpret_cast<int *>(src + 0x14);
        *g_009b33b0 = *reinterpret_cast<int *>(src + 0x18);
        *g_009b33a8 = *reinterpret_cast<int *>(src + 0x1c);
        *g_009b33b8 = *reinterpret_cast<int *>(src + 0x24);
        *g_009b33b4 = *reinterpret_cast<int *>(src + 0x28);
        *g_009b33bc = *reinterpret_cast<int *>(src + 0x20);
    }
}
