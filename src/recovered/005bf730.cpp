// ORIGINAL: 0x005BF730 BYTE_EXACT
// name      ?X_pops@@YAHPADPBDPADHPAUSprite@@HHP6AHXZ@Z
// size      64 bytes
// spans     0x005BF730-0x005BF770
// prototype 
// callers   0   call targets   2
// 0x005BF730  ?X_pops@@YAHPADPBDPADHPAUSprite@@HHP6AHXZ@Z  ->  ?X_pops@@YAHPADPBD0HPAUSprite@@HHP6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct Sprite;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl X_pops(char *, const char *, int length, char *, int, Sprite *,
                   int, int, int (__cdecl *)());
int __cdecl _strlen(const char *);

int __cdecl X_pops(char *a1, const char *a2, char *a3, int a4, Sprite *a5,
                   int a6, int a7, int (__cdecl *a8)()) {
    if (a3 == 0) {
        return -1;
    }
    return X_pops(a1, a2, _strlen(a3), a3, a4, a5, a6, a7, a8);
}
