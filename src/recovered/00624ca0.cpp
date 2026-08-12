// ORIGINAL: 0x00624CA0 BYTE_EXACT
// name      sub_624ca0
// size      40 bytes
// spans     0x00624CA0-0x00624CC8
// prototype 
// callers   0   call targets   1
// 0x00624CA0  sub_624ca0  ->  _sub_624ca0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
class Subject { public:
    void invoke(int, int, int, int, int, int);
    void forwarded(int, int, int, int, int);
};

void Subject::invoke(int a1, int a2, int a3, int a4, int a5, int a6) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    if (a6 == -1) {
        forwarded(a1, a2, a3, a4, a5);
    }
}
