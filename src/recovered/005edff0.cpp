// ORIGINAL: 0x005EDFF0 BYTE_EXACT
// name      ?UNK7@Win@@QAEXHHHH@Z
// size      45 bytes
// spans     0x005EDFF0-0x005EE01D
// prototype void (__thiscall ?UNK7@Win@@QAEXHHHH@Z)(Win* this, int xLeft, int yTop, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005EE011
// 0x005EDFF0  ?UNK7@Win@@QAEXHHHH@Z  ->  ?UNK7@Win@@QAEXHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *SetRectFn)(void*, int, int, int, int);

void Win::UNK7(int a1, int a2, int a3, int a4) {
    int rectangle[4];
    (*reinterpret_cast<SetRectFn *>(g_00669274))(
        rectangle, a1, a2, a1 + a3, a2 + a4);
}
