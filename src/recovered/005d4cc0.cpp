// 0x005D4CC0  sub_5d4cc0  ->  _sub_5d4cc0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl _free(void *);

class Sub5d4cc0 { public:
    void call(void *);
};

void Sub5d4cc0::call(void *a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x18) == 0) {
        char *argument = reinterpret_cast<char *>(a1);
        void *value = *reinterpret_cast<void **>(argument + 4);
        if (value) {
            _free(value);
            *reinterpret_cast<void **>(argument + 4) = 0;
        }
    }
}
