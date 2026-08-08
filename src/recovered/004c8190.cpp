// 0x004C8190  sub_4c8190  ->  _sub_4c8190
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl fn_0064557f(void *);

class Unknown_4c8190 { public:
    void close();
};

void Unknown_4c8190::close() {
    char *self = reinterpret_cast<char *>(this);
    void *first = *reinterpret_cast<void **>(self + 0x1c);
    if (first != 0) {
        fn_0064557f(first);
    }
    void *second = *reinterpret_cast<void **>(self + 0x24);
    if (second != 0) {
        fn_0064557f(second);
    }
}
