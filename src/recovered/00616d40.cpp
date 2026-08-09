// ORIGINAL: 0x00616D40
// 0x00616D40  sub_616d40  ->  _sub_616d40
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl _free(void *);

class Subject { public:
    void release();
};

void Subject::release() {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    char *self = reinterpret_cast<char *>(this);
    char *entry = *reinterpret_cast<char **>(self + 8);
    if (entry != 0) {
        void *allocation = *reinterpret_cast<void **>(entry + 0x18);
        if (allocation != 0) {
            _free(allocation);
            entry = *reinterpret_cast<char **>(self + 8);
            *reinterpret_cast<void **>(entry + 0x18) = 0;
        }
    }
}
