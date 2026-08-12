// ORIGINAL: 0x004C6000 BYTE_EXACT
// name      sub_4c6000
// size      42 bytes
// spans     0x004C6000-0x004C602A
// prototype 
// callers   0   call targets   0
// 0x004C6000  sub_4c6000  ->  _sub_4c6000
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__cdecl *Invoke)(int, int, int);

int __cdecl sub_4c6000(int a1, int a2, int a3) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    if (*g_0090db7c == 0) {
        return 1;
    }
    return (*reinterpret_cast<Invoke *>(g_0090db24))(a1, a2, a3);
}
