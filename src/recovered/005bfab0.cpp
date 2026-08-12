// ORIGINAL: 0x005BFAB0 BYTE_EXACT
// name      ?X_pop_ask@@YAHPBDHPADP6AHXZH@Z
// size      38 bytes
// spans     0x005BFAB0-0x005BFAD6
// prototype 
// callers   11   call targets   1
// 0x005BFAB0  ?X_pop_ask@@YAHPBDHPADP6AHXZH@Z  ->  ?X_pop_ask@@YAHPBDHPADP6AHXZH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl X_pop_ask(char* a1, int a2, char* a3,
                      int (__cdecl *a4)(), int a5) {
    return X_pop_ask(reinterpret_cast<char *>(g_009b8aa8),
                     a1, a2, a3, a4, a5);
}
