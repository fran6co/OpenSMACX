// ORIGINAL: 0x00493C70
// 0x00493C70  ?on_redraw@ProdPicker@@QAEXXZ  ->  ?on_redraw@ProdPicker@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void ProdPicker::on_redraw() {
    char *self = reinterpret_cast<char *>(this);
    for (int i = *reinterpret_cast<int *>(self + 0xa77c);
         i < *reinterpret_cast<int *>(self + 0xa77c) +
             *reinterpret_cast<int *>(self + 0xa82c);
         i++) {
        draw_item(i, 0);
    }
}
