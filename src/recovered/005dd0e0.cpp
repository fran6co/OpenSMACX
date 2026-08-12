// ORIGINAL: 0x005DD0E0 BYTE_EXACT
// 0x005DD0E0  ?write_cent_l@Buffer@@QAEHPADHHHHH@Z  ->  ?write_cent_l@Buffer@@QAEHPADHHHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int Buffer::write_cent_l(char *a1, int a2, int a3, int a4, int a5, int a6) {
    RECT r;
    r.left = a2;
    r.top = a3;
    r.right = a2 + a4;
    r.bottom = a3 + a5;
    return write_cent_l(a1, &r, a6);
}
