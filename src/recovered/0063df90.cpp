// ORIGINAL: 0x0063DF90 BYTE_EXACT
// name      sub_63df90
// size      77 bytes
// spans     0x0063DF90-0x0063DFDD
// prototype 
// callers   2   call targets   1
// 0x0063DF90  sub_63df90  ->  _sub_63df90
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
// The auto-generated declaration above this comment (scaffolding, prepended
// by the verifier, not repeated here) declares sub_643950 nullary because its
// arity is unrecorded in the catalogue. Respelling it WITH `extern "C"` here
// conflicts (C2733: second C linkage of overloaded function) because the
// scaffolding's own `extern "C"` declaration is already in scope; dropping
// the keyword on this second declaration keeps the inherited C linkage
// without repeating it, so the arity fix lands without a clash - the same
// pattern `src/recovered/004c74b0.cpp` uses for `_strcpy`.
//
// reads a stack parameter (not `ecx`) - a plain __cdecl free function taking
// one pointer argument, not the nullary guess above.
int __cdecl sub_643950(int, void *, void *);

struct Inner {
    uint8_t pad_0x14[0x14];
    int field_14;
};

typedef int(__cdecl *Callback)(int, int);

struct Outer {
    uint8_t pad_0[0x1C];
    Inner *field_1c;
    uint8_t pad_20[0x24 - 0x20];
    Callback field_24;
    int field_28;
};

extern "C" int __cdecl sub_63df90(Outer *a1) {
    int unused;
    if (a1 != 0) {
        if (a1->field_1c != 0) {
            if (a1->field_24 != 0) {
                if (a1->field_1c->field_14 != 0) {
                    sub_643950(a1->field_1c->field_14, a1, &unused);
                }
                a1->field_24(a1->field_28, reinterpret_cast<int>(a1->field_1c));
                a1->field_1c = 0;
                return 0;
            }
        }
    }
    return -2;
}
