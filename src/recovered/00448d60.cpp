// ORIGINAL: 0x00448D60 BYTE_EXACT
// name      ?close@FameWin@@QAEXXZ
// size      33 bytes
// spans     0x00448D60-0x00448D81
// prototype void (__thiscall ?close@FameWin@@QAEXXZ)(FameWin* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005D7470
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
