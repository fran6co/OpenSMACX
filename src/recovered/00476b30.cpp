// ORIGINAL: 0x00476B30 BYTE_EXACT
// name      ?clear_monuments@@YAXXZ
// size      49 bytes
// spans     0x00476B30-0x00476B61
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x00476B30  ?clear_monuments@@YAXXZ  ->  ?clear_monuments@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl clear_monuments() {
    char *monument = reinterpret_cast<char *>(g_0094cea0);
    do {
        char *item = monument - 0x488;
        int count = 0x15;
        do {
            *reinterpret_cast<int *>(item) = 0;
            item += 0x38;
        } while (--count);
        *reinterpret_cast<int *>(monument) = -1;
        monument += 0x4f4;
    } while (reinterpret_cast<int>(monument) <
             reinterpret_cast<int>(g_0094f640));
}
