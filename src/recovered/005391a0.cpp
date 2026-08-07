// 0x005391A0  sub_5391a0  ->  _sub_5391a0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_5391a0() {
    text_get();
    char *input = reinterpret_cast<char *>(*g_009b7d00);
    *reinterpret_cast<char *>(g_009b86a0) = 0;
    parse_string(input, reinterpret_cast<char *>(g_009b86a0));
}
