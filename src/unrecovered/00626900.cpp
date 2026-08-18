// ORIGINAL: 0x00626900 sub_626900 0x00626900-0x006269F3 FILE
// RULED-OUT: an early `if idx==target return 0` guard followed by a plain `for` loop with its OWN separate `if next==target return 0` deep inside (two textually-different early returns, mnemonic similarity 0.83, 9 edit blocks) vs. a top `if idx!=target` wrapping a `do`/`while idx!=target` loop, falling through to one shared trailing `return 0` (0.81, 8 edit blocks) - the original has exactly ONE physical epilogue shared by the top-of-function guard AND the loop's own back-edge (falls straight through at 0x00626949 to 0x0062694C with no jump), which only the do/while form expresses; kept that one even though its raw mnemonic-similarity score is marginally lower. Neither reached BYTE_EXACT; first divergence is prologue push/mov scheduling (#2).
// size      243 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00628AD0

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.
//
// IDA's own analysis (guess_type) reads this as `int __cdecl(int, int)`;
// the disassembly agrees - `[esp+0xc]` before any push is the second
// stack argument, `[esp+0x14]` after four pushes is the first - so the
// signature below spells that out instead of the fallback nullary head.

struct Pair8 {
    int lo;
    int hi;
};

extern int g_9bc010;
extern Pair8 *g_9bc018;
extern int *g_9bc02c;

int __cdecl fixed_div(int numerator, int denominator);

extern "C" int __cdecl sub_626900(int *a1, int a2) {
    int idx = a2;
    if (idx != g_9bc010) {
        int base = *a1;
        do {
            int next = base + idx;
            if (next > 3) {
                next = 0;
            } else if (next < 0) {
                next = 3;
            }
            int diff = g_9bc018[next].hi - g_9bc018[idx].hi;
            a1[1] = diff;
            if (diff != 0) {
                if (diff <= 0) {
                    return 0;
                }
                a1[2] = next;
                a1[7] = g_9bc018[idx].lo;
                int span = g_9bc018[next].lo - g_9bc018[idx].lo;
                a1[8] = span / a1[1];
                if (span < 0) {
                    span = -span;
                    a1[9] = -1;
                    a1[10] = 1 - a1[1];
                } else {
                    a1[9] = 1;
                    a1[10] = 0;
                }
                a1[11] = span % a1[1];
                a1[12] = a1[1];
                a1[13] = g_9bc02c[idx] * 0x10000;
                a1[14] = fixed_div(g_9bc02c[next] * 0x10000 - a1[13],
                                   diff * 0x10000);
                return 1;
            }
            idx = next;
        } while (idx != g_9bc010);
    }
    return 0;
}
