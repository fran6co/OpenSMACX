// 0x0050E9F0  sub_50e9f0  ->  _sub_50e9f0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" void __cdecl sub_50e9f0() {
    ((Time *)g_00915628)->~Time();
    ((Time *)g_00939e60)->~Time();
    ((Time *)g_00915658)->~Time();
    ((Time *)g_00939eb0)->~Time();
    ((Time *)g_00915688)->~Time();
}
