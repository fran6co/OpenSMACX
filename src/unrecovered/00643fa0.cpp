// ORIGINAL: 0x00643FA0
// name      sub_643fa0
// size      318 bytes
// spans     0x00643FA0-0x006440DE
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006439F0
// RULED-OUT: calling huft_build() directly by name - the catalogue records
//            it unmangled (`_huft_build`) with no CRT_SIGNATURES entry, so
//            declfix leaves it NULLARY and a real call is C2660, while
//            redeclaring it locally is C2733 (second C linkage). Routed the
//            call through a function-pointer cast instead, which compiles
//            but is a source-form change the comparison sees: the callee's
//            8 args are (b=code lengths, n, s, d, e, t, m, hn) matching a
//            custom zlib inflate_trees_fixed()/huft_build() pair, hn shared
//            across both calls at the same stack slot. Not reproduced: a
//            self-referential pointer the original stores at [esp+0x30]
//            before the first fill loop (role unknown - maybe part of an
//            allocator-context struct passed alongside `hn`).
//            Best reached: MISMATCH, edit_count 8, 301 bytes vs 318.
extern "C" int __cdecl sub_643fa0(int *bl, int *bd, void **tl, void **td) {
    typedef int (__cdecl *HuftBuildFn)(int *, int, int, void *, void *, void *, int *, int *);
    HuftBuildFn real_huft_build = (HuftBuildFn)huft_build;
    int lengths1[288];
    int lengths2[30];
    int hn;
    int i;

    if (*g_009c04a8 == 0) {
        for (i = 0; i < 144; i++) lengths1[i] = 8;
        for (i = 144; i < 256; i++) lengths1[i] = 9;
        for (i = 256; i < 280; i++) lengths1[i] = 7;
        for (i = 280; i < 288; i++) lengths1[i] = 8;
        hn = 0;
        *g_009c04a4 = 7;
        real_huft_build(lengths1, 288, 257, g_006a5a90, g_006a5b0c,
                         g_009bf40c, g_009c04a4, &hn);
        for (i = 0; i < 30; i++) lengths2[i] = 5;
        *g_009c04a0 = 5;
        real_huft_build(lengths2, 30, 0, g_006a5b88, g_006a5c00,
                         g_009bf408, g_009c04a0, &hn);
        *g_009c04a8 = 1;
    }
    *bl = *g_009c04a4;
    *bd = *g_009c04a0;
    *tl = (void *)*g_009bf40c;
    *td = (void *)*g_009bf408;
    return 0;
}
