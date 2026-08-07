// 0x005DA680  ?poly@Buffer@@QAEHPAUVert@@HH@Z  ->  ?poly@Buffer@@QAEHPAUVert@@HH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Buffer::poly(Vert *a1, int a2, int a3) {
    return polygon(this, a1, a2, a3);
}
