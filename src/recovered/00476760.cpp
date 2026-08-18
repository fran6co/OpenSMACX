// ORIGINAL: 0x00476760 ?close@MonuWin@@QAEXXZ 0x00476760-0x00476774 BYTE_EXACT
// size      20 bytes
// prototype void (__thiscall ?close@MonuWin@@QAEXXZ)(MonuWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40
// indirect  0x00476765
// 0x00476760  ?close@MonuWin@@QAEXXZ  ->  ?close@MonuWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MonuWin::close() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(self)->slot058();
    reinterpret_cast<GraphicWin *>(self)->close();
}
