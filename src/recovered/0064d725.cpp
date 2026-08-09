// ORIGINAL: 0x0064D725
// 0x0064D725  sub_64d725  ->  _sub_64d725
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
// __ld12cvt is called with 3 args (both incoming stack slots forwarded, plus
// a fixed address appended); the scaffold's bare-`ret`-floor zero-arg
// prototype is corrected here the same way as the _fopen/_fsopen thunk.
int __cdecl __ld12cvt(int, int, int);

// This CRT-style thunk forwards a1/a2 and appends a fixed struct address as
// a plain immediate; sub_64d725 itself is a bare-`ret` __cdecl floor too -
// [esp+0xc] read before any push shows two incoming int-sized params.
int __cdecl sub_64d725(int a1, int a2) {
    return __ld12cvt(a1, a2, 0x006A6928);
}
