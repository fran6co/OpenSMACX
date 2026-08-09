// ORIGINAL: 0x0064955B
// 0x0064955B  sub_64955b  ->  _sub_64955b
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl _flsall(int);

void __cdecl sub_64955b() {
    _flsall(1);
}
