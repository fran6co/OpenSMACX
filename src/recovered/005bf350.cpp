// ORIGINAL: 0x005BF350 ?X_pop@@YAHPADPBDPADHP6AHXZ@Z 0x005BF350-0x005BF384 BYTE_EXACT
// symbol    ?X_pop@@YAHPADPBD0HP6AHXZ@Z
// size      52 bytes
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480 0x006453E0
// 0x005BF350  ?X_pop@@YAHPADPBDPADHP6AHXZ@Z  ->  ?X_pop@@YAHPADPBD0HP6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl X_pop(char*, const char*, int length, char*, int,
                  int (__cdecl *)());
int __cdecl _strlen(const char*);

int __cdecl X_pop(char* a1, const char* a2, char* a3, int a4,
                  int (__cdecl *a5)()) {
    if (!a3) {
        return -1;
    }
    return X_pop(a1, a2, _strlen(a3), a3, a4, a5);
}
