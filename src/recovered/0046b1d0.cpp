// 0x0046B1D0  ?on_redraw@MapWin@@QAEXXZ  ->  ?on_redraw@MapWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MapWin::on_redraw() {
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) - 0x21a68)) {
        reinterpret_cast<MapWin *>(reinterpret_cast<char *>(this) - 0x21a6c)->draw_map(1);
    }
}
