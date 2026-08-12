// ORIGINAL: 0x00559060 BYTE_EXACT
// name      ?get_name@@YAPADH@Z
// size      50 bytes
// spans     0x00559060-0x00559092
// prototype int8* (__cdecl ?get_name@@YAPADH@Z)(int factionID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x00559060  ?get_name@@YAPADH@Z  ->  ?get_name@@YAPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct LeaderProfile {
    int32_t unk_000;                          // 0x00
    char pad_004[0x34 - 4];                   // 0x04
    char name_leader[24];                     // 0x34
    char pad_04C[0x59C - 0x34 - 24];          // 0x4C .. 0x59C
};

extern LeaderProfile g_leaders[];

char * __cdecl get_name(int a1) {
    *g_009bbff0 = 0;
    *g_009bbfec = g_leaders[a1].unk_000;
    return g_leaders[a1].name_leader;
}
