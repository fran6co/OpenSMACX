// ORIGINAL: 0x005D50F0
// name      sub_5d50f0
// size      325 bytes
// spans     0x005D50F0-0x005D5235
// prototype
// callers   0   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005D7030 0x005D7670 0x005DE8F0 0x005EC680 0x005EEF60
// PROPOSAL: extern "C" int __stdcall sub_5d50f0(9 ints) -> a __thiscall
//           member (9 forwarded ints, no separate `this`), evidence: `mov
//           esi, ecx` at entry immediately used as the receiver for
//           GraphicWin::close()/compute_min_size(), and `ret 0x24` (36
//           bytes = 9 ints) is consistent with either reading, but the raw
//           `ecx` use at entry only makes sense for thiscall/fastcall - a
//           free function declared `__thiscall` is refused outright by
//           VC6 (C4234), same as a `__thiscall` function-pointer type.
//           Expressed via an introduced `GWShim` class per the __thiscall
//           carve-out, since the scaffold's own GraphicWin class does not
//           declare this member. The class-close semicolon and the
//           out-of-line definition's open brace share ONE LINE below - not
//           style, the extractor's brace counter runs per line, and a
//           standalone closing line for the class closes the region it is
//           measuring before reaching the function at all, scoring the
//           whole thing NO_COMPILE with zero .text symbols found.
// RULED-OUT: reading `*(uint*)(base+0x98)` twice (once per bit test) added
//            an extra `mov` vs the original's single load reused by both
//            `test al,N`; caching it in a local did not change the
//            rebuilt bytes, so the remaining divergence is scheduling, not
//            this.
//            Best reached: MISMATCH, edit_count 5, 345 bytes vs 325,
//            mnemonic similarity ~0.92.
class GWShim {
public:
    int sub_5d50f0(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9);
}; int GWShim::sub_5d50f0(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9) {
    char *base = (char *)this;
    int result;

    ((GraphicWin *)this)->close();
    if ((a6 & 0x30000000) != 0) {
        *(int *)(base + 0x9cc) = *g_009b3394;
        *(int *)(base + 0x9d4) = *g_009b339c;
        *(int *)(base + 0x9d0) = *g_009b3398;
        *(int *)(base + 0x9d8) = *g_009b33a0;
        *(int *)(base + 0x9e8) = *g_009b33a4;
        *(int *)(base + 0x9e4) = *g_009b33a8;
        *(int *)(base + 0x9dc) = *g_009b33ac;
        *(int *)(base + 0x9e0) = *g_009b33b0;
        *(int *)(base + 0x9ec) = *g_009b33b8;
        *(int *)(base + 0x9f0) = *g_009b33b4;
        *(int *)(base + 0x9f4) = *g_009b33bc;
    }
    result = ((Win *)this)->UNK1(a1, a2, a3, a4, a5, a6, a7, a8, a9);
    if (result == 0) {
        *(int *)(base + 0x448) = (int)base;
        ((GraphicWin *)this)->compute_min_size();
        if ((a6 & 0x800) == 0) {
            ((Win *)this)->nonclient_to_client(&a3, &a4);
        } else {
            unsigned int flags98 = *(unsigned int *)(base + 0x98);
            if ((flags98 & 8) != 0) {
                a3 = a3 + *g_009b8dd4;
            }
            if ((flags98 & 4) != 0) {
                a4 = a4 + *g_009b8dd4;
            }
        }
        result = ((Buffer *)(base + 0x444))->init(a3, a4, 0, 0);
        if (result == 0) {
            ((Buffer *)(base + 0x444))->sync_to_palette((Palette *)g_009b8180);
            result = 0;
        }
    }
    return result;
}
