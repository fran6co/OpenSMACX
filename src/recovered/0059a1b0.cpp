// ORIGINAL: 0x0059A1B0 ??__EPaths@@YAXXZ 0x0059A1B0-0x0059A1BC BYTE_EXACT
// size      12 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645398
// 0x0059A1B0  ??__EPaths@@YAXXZ  ->  ??__EPaths@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl _atexit(void (__cdecl *)(void));

void __cdecl fn_0059a1b0() {
    _atexit((void (__cdecl *)(void))0x0059A1C0);
}
