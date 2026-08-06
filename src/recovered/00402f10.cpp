// 0x00402F10  ??__Eg_ALPHAMENU_WAVE@@YAXXZ  ->  ??__Eg_ALPHAMENU_WAVE@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" int __cdecl atexit(void (__cdecl *)(void));

inline void *__cdecl operator new(unsigned int, void *p) { return p; }

void __cdecl fn_00402f10() {
    new (g_006a7090) Wave;
    atexit(reinterpret_cast<void (__cdecl *)(void)>(g_00402f30));
}
