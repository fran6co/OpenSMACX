// ORIGINAL: 0x004964D0 BYTE_EXACT
// name      ?on_left_click@QuayleWin@@QAEXHH@Z
// size      99 bytes
// spans     0x004964D0-0x00496533
// prototype void (__thiscall ?on_left_click@QuayleWin@@QAEXHH@Z)(QuayleWin* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00
// 0x004964D0  ?on_left_click@QuayleWin@@QAEXHH@Z  ->  ?on_left_click@QuayleWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void QuayleWin::on_left_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);

    unsigned int bit = 1u << *g_00939284;
    if (*reinterpret_cast<unsigned int *>(g_0093e8ec) & bit) return;
    if (*g_0093e944 != 0) return;
    if (*g_0093e8c8 != 0) return;

    Spot *spot = reinterpret_cast<Spot *>(self + 0x214c);
    if (spot->check(a1, a2, &a2, &a1) + 1 != 0) {
        field_2144_ = a2;
    }

    reinterpret_cast<VCall *>(self)->slot062();
}
