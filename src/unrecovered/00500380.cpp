// ORIGINAL: 0x00500380
// name      sub_500380
// size      362 bytes
// spans     0x00500380-0x005004EA
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20 0x005BF1F0 0x00644F3A
// RULED-OUT: signature kept `void __cdecl sub_500380(int, int, int)` (was
//            void per Ghidra/asm - no eax set before ret) rather than the
//            brief's nullary head, since the three reads at [ebp+8/c/10]
//            with no `this` use are plain stack args. Body reproduces the
//            two early-return conditions inside the loop's abitily/range
//            check (`return`, matching Ghidra, not `continue` - both jump
//              to the function epilogue, not the loop latch) and the
//            candidate-selection guard (has_tech || table-owner match).
//            0.824 mnemonic similarity; first divergence at #5 is the
//            callee-saved register push order in the prologue (ebx/esi/edi
//            pushed at different points around the `mov esi,[ebp+8]`),
//            which register allocation controls and no source reshuffle
//            tried here changed.

typedef unsigned char uint8_t;

bool has_tech(int, int);
extern "C" int abs(int);
int has_abil(int, int);

static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00949740 = (int *)0x00949740;
static int *const g_009497c8 = (int *)0x009497C8;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static uint8_t *const g_0094988c = (uint8_t *)0x0094988C;
static char *const g_0094a30c = (char *)0x0094A30C;
static char *const g_00952832 = (char *)0x00952832;
static uint8_t *const g_0097d044 = (uint8_t *)0x0097D044;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a6580 = (int *)0x009A6580;

extern "C" void __cdecl sub_500380(int a1, int a2, int a3) {
    bool skip;
    if (has_tech(*g_009497c8, a1)) {
        skip = true;
    } else if (*g_009a6580 >= 0) {
        skip = (g_0097d044[*g_009a6580 * 0x134] == a1);
    } else {
        skip = false;
    }

    int threshold;
    if (skip) {
        threshold = *g_00949870;
        if (threshold <= *g_00949874) {
            threshold = *g_00949874;
        }
    } else {
        threshold = *g_00949740;
    }

    int count = *g_009a64c8;
    if (count > 0) {
        char *rec = g_00952832;
        for (int i = 0; i < count; ++i, rec += 0x34) {
            if (*reinterpret_cast<uint8_t *>(rec + 4) != a1) {
                continue;
            }
            int abil = *reinterpret_cast<short *>(rec);
            if (!has_abil(abil, 0x10)) {
                continue;
            }
            if (*reinterpret_cast<uint8_t *>(rec - 6) & 0x20) {
                continue;
            }
            if (*reinterpret_cast<uint8_t *>(rec + 0x1e)) {
                continue;
            }
            int size1 = *reinterpret_cast<short *>(rec - 0xa);
            int size2 = *reinterpret_cast<short *>(rec - 8);
            int dx = abs(a2 - size1);
            if (!(*g_0094988c & 1) && dx > *g_0068faf0) {
                dx = *g_00949870 - dx;
            }
            int dy = abs(a3 - size2);
            if ((dy + dx) >> 1 > threshold) {
                continue;
            }
            int idx = *g_0068faf0 * size2 + (size1 >> 1);
            char *base = g_0094a30c + idx * 0x2c;
            uint8_t flags8 = *reinterpret_cast<uint8_t *>(base + 8);
            if (flags8 & 1) {
                if ((*reinterpret_cast<uint8_t *>(base + 2) & 0xf) < 8) {
                    return;
                }
            }
            if (*reinterpret_cast<unsigned int *>(base + 8) & 0x40000) {
                return;
            }
        }
    }
}
