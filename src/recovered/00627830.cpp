// 0x00627830  ?pop_ask@@YAHPADPADHP6AHXZH@Z  ->  ?pop_ask@@YAHPAD0HP6AHXZH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl pop_ask(char *a1, char *a2, int a3, int (__cdecl *a4)(), int a5) {
    return pop_ask(a1, a2, a3, 0, a4, a5);
}
