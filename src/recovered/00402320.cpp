// ORIGINAL: 0x00402320 sub_402320 0x00402320-0x00402349 BYTE_EXACT
// size      41 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644EF2
// 0x00402320  sub_402320  ->  _sub_402320
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__cdecl *FreeFn)(void *);

extern "C" int __cdecl sub_402320(int param_1) {
    if (param_1 != 0) {
        if (*g_009b3374 == 0) {
            ((FreeFn)free)((void *)param_1);
        }
        *g_009b3374 = 0;
    }
    if (0) {
        return param_1;
    }
}
