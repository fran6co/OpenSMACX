// ORIGINAL: 0x00646010
// 0x00646010  _srand  ->  _srand
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void *__cdecl __getptd();

void __cdecl _srand(unsigned int seed) {
    *reinterpret_cast<unsigned int *>(reinterpret_cast<char *>(__getptd()) + 0x14) = seed;
}
