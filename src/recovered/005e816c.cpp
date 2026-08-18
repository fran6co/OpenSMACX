// ORIGINAL: 0x005E816C ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z 0x005E816C-0x005E81BC BYTE_EXACT
// size      80 bytes
// prototype int (__thiscall ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z)(Sprite* this, Buffer*, int, int, int, int, int, unsigned int8*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E81BC
// 0x005E816C  ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z  ->  ?draw_dest_unk2@Sprite@@QAEHPAUBuffer@@HHHHHPAE@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Sprite::draw_dest_unk2(Buffer * a1, int a2, int a3, int a4, int a5, int a6, unsigned char * a7) {
    int old1 = *g_00696d18;
    int old2 = *g_00696d1c;
    *g_00696d18 = a5;
    *g_00696d1c = a6;
    int result = draw_dest_unk2(a1, a2, a3, a4, a7);
    *g_00696d1c = old2;
    *g_00696d18 = old1;
    return result;
}
