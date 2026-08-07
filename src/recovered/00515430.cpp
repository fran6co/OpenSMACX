// 0x00515430  sub_515430  ->  _sub_515430
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" void __cdecl sub_515430() {
    *g_00939ed8 = 0;
    desktop_init(1);
}
