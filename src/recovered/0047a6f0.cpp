// ORIGINAL: 0x0047A6F0 BYTE_EXACT
// name      ?UNK3@MultiWin@@QAEHHHPAURECT@@@Z
// size      46 bytes
// spans     0x0047A6F0-0x0047A71E
// prototype int (__thiscall ?UNK3@MultiWin@@QAEHHHPAURECT@@@Z)(MultiWin* this, int, int, RECT*)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FA960
// 0x0047A6F0  ?UNK3@MultiWin@@QAEHHHPAURECT@@@Z  ->  ?UNK3@MultiWin@@QAEHHHPAURECT@@@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int MultiWin::UNK3(int a1, int a2, RECT* a3) {
    return reinterpret_cast<Spot *>(this)->add(
        a1, a2, a3->left, a3->top,
        a3->right - a3->left, a3->bottom - a3->top);
}
