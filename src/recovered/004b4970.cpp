// ORIGINAL: 0x004B4970
// 0x004B4970  ?say_orders@@YAXH@Z  ->  ?say_orders@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl say_orders(int a1) {
    say_orders(reinterpret_cast<char *>(g_009b86a0), a1);
}
