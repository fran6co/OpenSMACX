// ORIGINAL: 0x0050E9B0 BYTE_EXACT
// 0x0050E9B0  sub_50e9b0  ->  _sub_50e9b0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

#include <new>

extern "C" int __cdecl atexit(void (__cdecl *)(void));

extern "C" void __cdecl sub_50e9b0() {
    new (g_00915688) Time();
    new (g_00939eb0) Time();
    new (g_00915658) Time();
    new (g_00939e60) Time();
    new (g_00915628) Time();
    atexit(reinterpret_cast<void (__cdecl *)(void)>(g_0050e9f0));
}
