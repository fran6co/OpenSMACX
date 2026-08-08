// 0x00539970  sub_539970  ->  _sub_539970
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_539970() {
    if (reinterpret_cast<Win *>(g_00733990)->is_visible()) {
        reinterpret_cast<DiploPop *>(g_00733990)->draw_info();
        reinterpret_cast<GraphicWin *>(g_00733990)->
            soft_update(reinterpret_cast<RECT *>(g_00735a84));
    }
}
