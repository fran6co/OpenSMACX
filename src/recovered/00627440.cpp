// ORIGINAL: 0x00627440 BYTE_EXACT
// 0x00627440  ?pops@@YAHPADPADPADHPAUSprite@@P6AHXZ@Z  ->  ?pops@@YAHPAD00HPAUSprite@@P6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct Sprite;

// ---- callees, declared and never defined (a definition would be inlined) ----
// Real types (see 0x00627190, same fix): `strlen`, one leading underscore,
// and the 9-arg `pops` overload this 6-arg one forwards into.
extern "C" unsigned int strlen(const char *);
int __cdecl pops(char *, char *, int, char *, int, Sprite *, int, int,
                 int (__cdecl *)());

#pragma function(strlen)

int __cdecl pops(char *a1, char *a2, char *a3, int a4, Sprite *a5,
                  int (__cdecl *a6)()) {
    if (a3 == 0) {
        return -1;
    }
    int len = strlen(a3);
    return pops(a1, a2, len, a3, a4, a5, 1, 1, a6);
}
