// ORIGINAL: 0x0048C320 BYTE_EXACT
// name      ?popt@@YAHPADPBDHPADH@Z
// size      51 bytes
// spans     0x0048C320-0x0048C353
// prototype 
// callers   2   call targets   3
// 0x0048C320  ?popt@@YAHPADPBDHPADH@Z  ->  ?popt@@YAHPADPBDH0H@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl popt(char *a1, const char *a2, int a3, char *a4, int a5) {
    popups_tutorial();
    int result = X_pop(a1, a2, a3, a4, a5, 0);
    popups_normal();
    return result;
}
