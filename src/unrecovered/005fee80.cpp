// ORIGINAL: 0x005FEE80 ?UNK4@Palette@@QAEHPAXHHHHH@Z 0x005FEE80-0x005FEFE7
// TRIED: `int i` reused across the two fill loops is a VC6 for-scope leak (C2374), so the loop counters are named `i0`/`i1`. The blend loop's R/G/B channel math and the `get_nearest_palette_index` call are transcribed directly from the Ghidra pseudocode (the CONCAT31 casts there are just "pass the low byte", nothing else). 0.81 mnemonic similarity; first divergence at #3 is in the prologue stack-frame setup for the two 0x400-byte local copies, not chased further.
// size      359 bytes
// prototype int (__thiscall ?UNK4@Palette@@QAEHPAXHHHHH@Z)(Palette* this, void*, int, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FF470 0x00645930

typedef unsigned char uint8_t;

extern "C" void *memcpy(void *, const void *, unsigned int);

class Palette {
public:
    int UNK4(void *a1, int a2, int a3, int a4, int a5, int a6);
    int get_nearest_palette_index(uint8_t, uint8_t, uint8_t, int);
};

static int *const g_009b8178 = (int *)0x009B8178;

int Palette::UNK4(void *a1, int a2, int a3, int a4, int a5, int a6) {
    if (a1 == 0) {
        return 0x10;
    }
    if (*g_009b8178 == 0) {
        return 7;
    }
    if (a2 == 0) {
        return 0x10;
    }

    uint8_t *dest = reinterpret_cast<uint8_t *>(a2);
    for (int i0 = 0; i0 < a3; ++i0) {
        dest[i0] = static_cast<uint8_t>(i0);
    }

    int hi = a3 + a4;
    for (int i1 = hi; i1 < 0x100; ++i1) {
        dest[i1] = static_cast<uint8_t>(i1);
    }

    uint8_t selfCopy[0x400];
    uint8_t srcCopy[0x400];
    memcpy(selfCopy, this, 0x400);
    memcpy(srcCopy, a1, 0x400);

    if (a3 < hi) {
        int divisor = a5 + a6;
        int i = a3;
        do {
            uint8_t r = static_cast<uint8_t>(
                (static_cast<unsigned int>(srcCopy[i * 4]) * a6 +
                 static_cast<unsigned int>(selfCopy[i * 4]) * a5) / divisor);
            selfCopy[i * 4] = r;

            uint8_t g = static_cast<uint8_t>(
                (static_cast<unsigned int>(srcCopy[i * 4 + 1]) * a6 +
                 static_cast<unsigned int>(selfCopy[i * 4 + 1]) * a5) / divisor);
            selfCopy[i * 4 + 1] = g;

            int bSum = static_cast<unsigned int>(srcCopy[i * 4 + 2]) * a6 +
                       static_cast<unsigned int>(selfCopy[i * 4 + 2]) * a5;
            int b = bSum / divisor;
            selfCopy[i * 4 + 2] = static_cast<uint8_t>(b);

            uint8_t idx = static_cast<uint8_t>(
                get_nearest_palette_index(r, g, static_cast<uint8_t>(b), 0));

            ++i;
            dest[i - 1] = idx;
        } while (i < hi);
    }

    return 0;
}
