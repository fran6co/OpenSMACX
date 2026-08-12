// ORIGINAL: 0x0057DF00 BYTE_EXACT
// name      ?clear_bunglist@@YAXH@Z
// size      47 bytes
// spans     0x0057DF00-0x0057DF2F
// prototype 
// callers   3   call targets   0
// 0x0057DF00  ?clear_bunglist@@YAXH@Z  ->  ?clear_bunglist@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl clear_bunglist(int a1) {
    uint8_t *entry =
        reinterpret_cast<uint8_t *>(g_0094b55d) + a1 * 0x200;
    int count = 0x40;
    uint8_t clear_value = 0xff;
    do {
        *reinterpret_cast<int *>(entry - 5) = -1;
        entry[-1] = clear_value;
        entry[0] = clear_value;
        entry[1] = clear_value;
        entry[2] = clear_value;
        entry += 8;
    } while (--count);
}
