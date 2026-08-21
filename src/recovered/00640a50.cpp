// ORIGINAL: 0x00640A50 sub_640a50 0x00640A50-0x00640A63 BYTE_EXACT
// symbol    ?sub_640a50@@YAHHHH@Z
// size      19 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00646ED0
// 0x00640A50  sub_640a50  ->  _sub_640a50
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl _calloc(int, int);

int __cdecl sub_640a50(int a1, int a2, int a3) {
    return _calloc(a2, a3);
}
