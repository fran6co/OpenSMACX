// ORIGINAL: 0x005E6652 BYTE_EXACT
// name      ?draw_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z
// size      80 bytes
// spans     0x005E6652-0x005E66A2
// prototype int (__thiscall ?draw_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z)(Sprite* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   1   call targets   1
// 0x005E6652  ?draw_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z  ->  ?draw_dest@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Sprite::draw_dest(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7) {
    int saved18 = *g_00696d18;
    int saved1c = *g_00696d1c;
    *g_00696d18 = a5;
    *g_00696d1c = a6;
    int ret = draw_dest(a1, a2, a3, a4, a7);
    *g_00696d1c = saved1c;
    *g_00696d18 = saved18;
    return ret;
}
