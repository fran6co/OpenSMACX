// ORIGINAL: 0x00645646
// 0x00645646  _fopen  ->  _fopen
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
// _fopen has a bare `ret` (cdecl purges nothing), which the emitter floors to
// zero args. This is the real CRT shape: fopen(filename, mode) tail-calls
// _fsopen(filename, mode, _SH_DENYNO) with _SH_DENYNO == 0x40.
int __cdecl __fsopen(const char *, const char *, int);

int __cdecl _fopen(const char *filename, const char *mode) {
    return __fsopen(filename, mode, 0x40);
}
