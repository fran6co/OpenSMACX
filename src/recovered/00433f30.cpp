// ORIGINAL: 0x00433F30 BYTE_EXACT
// name      ?on_scrolled@SelectPartWin@@QAEXHH@Z
// size      31 bytes
// spans     0x00433F30-0x00433F4F
// prototype void (__thiscall ?on_scrolled@SelectPartWin@@QAEXHH@Z)(SelectPartWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00433F45
// 0x00433F30  ?on_scrolled@SelectPartWin@@QAEXHH@Z  ->  ?on_scrolled@SelectPartWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void SelectPartWin::on_scrolled(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0x2b60) = *reinterpret_cast<int *>(self + 0x2b6c) * a2;
    reinterpret_cast<VCall *>(self)->slot062();
}
