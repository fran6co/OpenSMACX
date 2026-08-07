// 0x0063BAC0  ?cos@@YAHHH@Z  ->  ?cos@@YAHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl cos(int a1, int a2) {
    return sin(a1 + 0x3fffffff, a2);
}
