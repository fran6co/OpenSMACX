// ORIGINAL: 0x00440F30
// 0x00440F30  sub_440f30  ->  _sub_440f30
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" int __cdecl _atexit(void (__cdecl *)());

void __cdecl sub_440f30() {
    fn_00645f8e(g_00737cd8, 0x30, 0x40,
                reinterpret_cast<void (*)(void*)>(g_004450b0),
                reinterpret_cast<void (*)(void*)>(g_00406820));
    _atexit(reinterpret_cast<void (__cdecl *)()>(g_00440f60));
}
