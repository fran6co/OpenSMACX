// ORIGINAL: 0x0064603F BYTE_EXACT
// name      __fwrite
// size      47 bytes
// spans     0x0064603F-0x0064606E
// prototype int (__cdecl __fwrite)(void* Str, size_t Size, size_t Count, FILE* File)
// callers   11   call targets   3
// 0x0064603F  __fwrite  ->  __fwrite
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl __lock_file(void *);
void __cdecl __unlock_file(void *);
int __cdecl _fwrite(void *, uint32_t, uint32_t, void *);

int __cdecl __fwrite(void *a1, uint32_t a2, uint32_t a3, void *a4) {
    __lock_file(a4);
    int result = _fwrite(a1, a2, a3, a4);
    __unlock_file(a4);
    return result;
}
