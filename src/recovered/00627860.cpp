// ORIGINAL: 0x00627860
// 0x00627860  ?pop_ask@@YAHPADHP6AHXZH@Z  ->  ?pop_ask@@YAHPADHP6AHXZH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl pop_ask(char *a1, int a2, int (__cdecl *a3)(), int a4) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    return pop_ask(reinterpret_cast<char *>(g_009b8aa8), a1, a2, 0, a3, a4);
}
