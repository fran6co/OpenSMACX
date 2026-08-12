// ORIGINAL: 0x0064557F BYTE_EXACT
// name      ??3@YAXPAX@Z
// size      11 bytes
// spans     0x0064557F-0x0064558A
// prototype 
// callers   115   call targets   1
// 0x0064557F  ??3@YAXPAX@Z  ->  ??3@YAXPAX@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl _free(void *p);

void __cdecl fn_0064557f(void *p) {
    _free(p);
}
