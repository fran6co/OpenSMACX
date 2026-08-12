// ORIGINAL: 0x005AD9A0 BYTE_EXACT
// 0x005AD9A0  ?close@ReplayWin@@QAEXXZ  ->  ?close@ReplayWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void ReplayWin::close() {
    reinterpret_cast<GraphicWin *>(this)->close();
    reinterpret_cast<MapWin *>(reinterpret_cast<char *>(this) + 0x2320)->close();
    void **field = reinterpret_cast<void **>(reinterpret_cast<char *>(this) + 0x2324);
    if (*field != 0) {
        free(*field);
        *field = 0;
    }
}
