// ORIGINAL: 0x005D54A0 BYTE_EXACT
// 0x005D54A0  sub_5d54a0  ->  _sub_5d54a0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sub5d7a30 { public:
    int call(int, int, int, int);
};

class Sub5d54a0 { public:
    int call(int, int, int, int);
};

int Sub5d54a0::call(int a1, int a2, int a3, int a4) {
    char *self = reinterpret_cast<char *>(this);
    return reinterpret_cast<Sub5d7a30 *>(self + 0x444)->
        call(a1, a2, a3, a4);
}
