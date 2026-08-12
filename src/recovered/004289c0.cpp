// ORIGINAL: 0x004289C0 BYTE_EXACT
// name      ?close@Credits@@QAEXXZ
// size      41 bytes
// spans     0x004289C0-0x004289E9
// prototype void (__thiscall ?close@Credits@@QAEXXZ)(Credits* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00616780
// 0x004289C0  ?close@Credits@@QAEXXZ  ->  ?close@Credits@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Credits::close() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<Time *>(self + 0x2D44)->close();
    reinterpret_cast<GraphicWin *>(this)->close();
    reinterpret_cast<VCall *>(self + 0x21F8)->slot090();
}
