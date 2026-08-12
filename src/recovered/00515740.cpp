// ORIGINAL: 0x00515740 BYTE_EXACT
// 0x00515740  ?toggle_dest@Console@@QAEXXZ  ->  ?toggle_dest@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::toggle_dest() {
    int *base = *reinterpret_cast<int **>(g_007d3c3c);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(base) + 0x1dd70) ^= 0x40000;
    draw_map(1);
}
