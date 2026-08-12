// ORIGINAL: 0x005BFB40 BYTE_EXACT
// name      ?X_pop_ask@@YAHPADPBDPADP6AHXZH@Z
// size      52 bytes
// spans     0x005BFB40-0x005BFB74
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BFBC0 0x006453E0
// 0x005BFB40  ?X_pop_ask@@YAHPADPBDPADP6AHXZH@Z  ->  ?X_pop_ask@@YAHPADPBD0P6AHXZH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl _strlen(const char *);

int __cdecl X_pop_ask(char *a1, const char *a2, char *a3,
                      int (__cdecl *a4)(), int a5) {
    if (a3 == 0) {
        return -1;
    }
    return X_pop_ask(a1, a2, _strlen(a3), a3, a4, a5);
}
