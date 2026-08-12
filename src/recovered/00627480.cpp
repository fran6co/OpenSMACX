// ORIGINAL: 0x00627480 BYTE_EXACT
// 0x00627480  ?pops@@YAHPADPADPADHPAUSprite@@HHP6AHXZ@Z  ->  ?pops@@YAHPAD00HPAUSprite@@HHP6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Named `_strlen`, not `strlen`: the front end recognises the spelling
// `strlen` as an intrinsic and inlines it (`repne scasb`) regardless of
// this declaration, which the original does not do - it calls `_strlen`
// out of line. Both spellings link to the same C symbol; only this one
// keeps the call.
extern "C" unsigned int _strlen(const char *);

int __cdecl pops(char * a1, char * a2, char * a3, int a4, Sprite * a5, int a6, int a7, int (__cdecl *a8)()) {
    if (a3 == 0) {
        return -1;
    }
    return pops(a1, a2, _strlen(a3), a3, a4, a5, a6, a7, a8);
}
