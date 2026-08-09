// ORIGINAL: 0x005590A0
// 0x005590A0  ?get_title@@YAPADH@Z  ->  ?get_title@@YAPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

struct LeaderProfile {
    int32_t unk_000;                    // 0x00
    char pad_004[0x34 - 4];             // 0x04 .. 0x34
    char name_leader[24];               // 0x34
    char title_leader[24];              // 0x4C
    char pad_064[0x3AC - 0x64];         // 0x64 .. 0x3AC
    char insult_leader[24];             // 0x3AC
    char pad_3C4[0x59C - 0x3C4];        // 0x3C4 .. 0x59C
};

extern LeaderProfile g_leaders[];

char * __cdecl get_title(int a1) {
    *g_009bbff0 = 0;
    *g_009bbfec = g_leaders[a1].unk_000;
    return g_leaders[a1].title_leader;
}
