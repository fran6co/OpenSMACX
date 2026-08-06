// 0x00402320  sub_402320  ->  _sub_402320
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

#pragma warning(disable: 4716)

typedef int (__cdecl *free_fn_t)(int);

extern "C" int __cdecl sub_402320(int param_1) {
    if (param_1 != 0) {
        if (*g_009b3374 == 0) {
            ((free_fn_t)free)(param_1);
        }
        *g_009b3374 = 0;
    }
}
