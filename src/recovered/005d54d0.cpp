// ORIGINAL: 0x005D54D0 sub_5d54d0 0x005D54D0-0x005D54F2 BYTE_EXACT
// symbol    ?invoke@Subject@@QAEHHHHH@Z
// size      34 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7AE0
// 0x005D54D0  sub_5d54d0  ->  _sub_5d54d0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
class Callee { public:
    int invoke(int, int, int, int);
};
class Subject { public:
    int invoke(int, int, int, int);
};

int Subject::invoke(int a1, int a2, int a3, int a4) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    Callee *callee = reinterpret_cast<Callee *>(reinterpret_cast<char *>(this) + 0x444);
    return callee->invoke(a1, a2, a3, a4);
}
