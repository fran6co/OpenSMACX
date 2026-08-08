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
