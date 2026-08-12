// ORIGINAL: 0x004C7270 BYTE_EXACT
// 0x004C7270  sub_4c7270  ->  _sub_4c7270
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void * __cdecl _memset(void *, int, unsigned int);

class Subject { public:
    Subject *initialize();
};

Subject *Subject::initialize() {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    char *self = reinterpret_cast<char *>(this);
    _memset(self + 0xc, 0, 0x104);
    *reinterpret_cast<int *>(self + 4) = 0;
    *reinterpret_cast<int *>(self) = 0;
    *reinterpret_cast<int *>(self + 8) = 1;
    return this;
}
