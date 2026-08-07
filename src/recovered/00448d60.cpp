// 0x00448D60  ?close@FameWin@@QAEXXZ  ->  ?close@FameWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void FameWin::close() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<Buffer *>(self + 0xa91c)->close();
    reinterpret_cast<VCall *>(this)->slot058();
    reinterpret_cast<GraphicWin *>(this)->close();
}
