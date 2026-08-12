// ORIGINAL: 0x006354F0 BYTE_EXACT
// name      sub_6354f0
// size      19 bytes
// spans     0x006354F0-0x00635503
// prototype 
// callers   0   call targets   1
// 0x006354F0  sub_6354f0  ->  _sub_6354f0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __stdcall sub_635510(int, int, int, int);

int __stdcall sub_6354f0(int a1) {
    return sub_635510(0, 0, a1, 1);
}
