// ORIGINAL: 0x004964A0 BYTE_EXACT
// name      ?close@QuayleWin@@QAEXXZ
// size      33 bytes
// spans     0x004964A0-0x004964C1
// prototype void (__thiscall ?close@QuayleWin@@QAEXXZ)(QuayleWin* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00609F20
// indirect  0x004964B2
// 0x004964A0  ?close@QuayleWin@@QAEXXZ  ->  ?close@QuayleWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void QuayleWin::close() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<ListBox *>(self + 0x2158)->close();
    reinterpret_cast<VCall *>(self)->slot058();
    reinterpret_cast<GraphicWin *>(self)->close();
}
