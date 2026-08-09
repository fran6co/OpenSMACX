// ORIGINAL: 0x00646178
// 0x00646178  __fread  ->  __fread
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct FILE;
typedef unsigned int size_t;

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl __lock_file(FILE *);
void __cdecl __unlock_file(FILE *);
size_t __cdecl _fread(void *, size_t, size_t, FILE *);

size_t __cdecl __fread(void *a1, size_t a2, size_t a3, FILE *a4) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    __lock_file(a4);
    size_t result = _fread(a1, a2, a3, a4);
    __unlock_file(a4);
    return result;
}
