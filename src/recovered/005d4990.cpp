// 0x005D4990  sub_5d4990  ->  _sub_5d4990
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
// Redeclared with its real parameter: the scaffold's blind `_free()` stub
// takes zero arguments and rejects the actual call with C2660 (see
// src/recovered/005d4cc0.cpp, 0064557f.cpp, 00640a70.cpp for the same fix).
void __cdecl _free(void *);

// `ecx+0x18` is read with no stack access at entry, so the receiver is a
// real `this`: this is a `__thiscall` member. `ret 4` cleans exactly one
// explicit stack argument, and `eax` is never set before it - the pinned
// `int __stdcall(int)` signature is a placeholder guess, not what the
// bytes show.
class Sub5d4990 { public:
    void call(void *a1);
};

void Sub5d4990::call(void *a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x18) == 0) {
        char *arg = reinterpret_cast<char *>(a1);
        void *v1 = *reinterpret_cast<void **>(arg + 4);
        if (v1 != 0) {
            _free(v1);
            *reinterpret_cast<void **>(arg + 4) = 0;
        }
        void *v2 = *reinterpret_cast<void **>(arg + 8);
        if (v2 != 0) {
            _free(v2);
            *reinterpret_cast<void **>(arg + 8) = 0;
        }
    }
}
