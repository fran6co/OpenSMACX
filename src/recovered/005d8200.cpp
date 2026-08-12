// ORIGINAL: 0x005D8200 BYTE_EXACT
// 0x005D8200  ?set_clip@Buffer@@QAEHHHHH@Z  ->  ?set_clip@Buffer@@QAEHHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *SetRectFn)(RECT *, int, int, int, int);

int Buffer::set_clip(int a1, int a2, int a3, int a4) {
    RECT r;
    (*reinterpret_cast<SetRectFn *>(g_00669274))(&r, a1, a2, a1 + a3, a2 + a4);
    return set_clip(&r);
}
