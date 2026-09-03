// ORIGINAL: 0x0064D77E sub_64d77e 0x0064D77E-0x0064D7AB BYTE_EXACT
// symbol    ?sub_64d77e@@YAHPAX0@Z
// size      45 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064D73B 0x0064F524
// 0x0064D77E  sub_64d77e  ->  _sub_64d77e
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl ___strgtold12(int *, void **, void *, int, int, int, int);
int __cdecl sub_64d73b(int *, void *);

int __cdecl sub_64d77e(void *a1, void *a2) {
    int value[3];
    ___strgtold12(value, &a2, a2, 0, 0, 0, 0);
    return sub_64d73b(value, a1);
}
