// ORIGINAL: 0x006239B0
// 0x006239B0  ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@HPAUVert@@H@Z  ->  ?draw_trans@Texture@@QAEXPAUBuffer@@PAUVert@@H1H@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Texture::draw_trans(Buffer* a1, Vert* a2, int a3, Vert* a4, int a5) {
    if (a5 == -1) {
        draw_trans(a1, a2, a3, a4);
    }
}
