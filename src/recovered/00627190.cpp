// 0x00627190  ?pop@@YAHPADPADPADHP6AHXZ@Z  ->  ?pop@@YAHPAD00HP6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct Sprite;

// ---- callees, declared and never defined (a definition would be inlined) ----
// NOTE: the stale placeholders this scaffold shipped with (`_strlen()` with
// no parameter, opaque `int8*` for both PAD and the Sprite* slot) did not
// match the mangled types. The live scaffolding derives the real ones:
// `strlen` (CRT name, one leading underscore added by cdecl decoration),
// and `pops`'s real parameter types below.
extern "C" unsigned int strlen(const char *);
int __cdecl pops(char *, char *, int, char *, int, Sprite *, int, int,
                 int (__cdecl *)());

// `strlen` is an MSVC intrinsic at /O2 (`repne scasb` inline) unless told
// otherwise - this forces the real call the original disassembly shows.
#pragma function(strlen)

int __cdecl pop(char *a1, char *a2, char *a3, int a4, int (__cdecl *a5)()) {
    if (a3 == 0) {
        return -1;
    }
    int len = strlen(a3);
    return pops(a1, a2, len, a3, a4, 0, 1, 1, a5);
}
