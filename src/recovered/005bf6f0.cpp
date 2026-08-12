// ORIGINAL: 0x005BF6F0 BYTE_EXACT
// 0x005BF6F0  ?X_pops@@YAHPADPBDPADHPAUSprite@@P6AHXZ@Z  ->  ?X_pops@@YAHPADPBD0HPAUSprite@@P6AHXZ@Z
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
                   int (__cdecl *a6)()) {
    if (a3 == 0) {
        return -1;
    }
    int len = _strlen(a3);
    return X_pops(a1, a2, len, a3, a4, a5, 1, 1, a6);
}
