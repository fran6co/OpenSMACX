// ORIGINAL: 0x005061B0 FILE
// name      ?battle_kill_chaff@@YAXHPAHPAHPAHHH@Z
// size      242 bytes
// spans     0x005061B0-0x005062A2
// prototype void (__cdecl ?battle_kill_chaff@@YAXHPAHPAHPAHHH@Z)(int, int*, int*, int*, int, int)
// callers   ?   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00505D80
// RULED-OUT: nothing tried beyond the direct transcription below - landed
//            at first compiling candidate given batch size. Mnemonic
//            similarity 0.93, first divergence is a `movsx` the rebuilt
//            body splits into two `mov`s (short-to-int widening scheduled
//            differently around the record-base computation, #25).

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.
//
// Two distinct short-stride tables (record stride 0x1a shorts = 0x34
// bytes): g_95285a is the "next free slot" link chased at the top, and
// g_952832/g_952858/g_952836/g_952838 are per-record fields read inside
// the main loop at the SAME 0x34-byte stride. g_9ab892/g_9ab88f are a
// second table (per-faction), indexed by the short read out of g_952832,
// at its own 0x34-byte stride.

extern short g_952832[];
extern short g_952858[];
extern short g_95285a[];
extern unsigned char g_952836[];
extern unsigned char g_952838[];
extern unsigned char g_9ab892[];
extern unsigned char g_9ab88f[];

int __cdecl battle_kill(int, int *, int *, int *, int, int);

void __cdecl battle_kill_chaff(int a1, int * a2, int * a3, int * a4, int a5, int a6) {
    if (a1 < 0) {
        return;
    }
    int idx = a1;
    short next = g_95285a[idx * 0x1a];
    while (next >= 0) {
        idx = next;
        next = g_95285a[idx * 0x1a];
    }
    if (idx < 0) {
        return;
    }
    int killerId = a5;
    int cur = idx;
    for (;;) {
        int record = cur * 0x34;
        short faction = g_952832[cur * 0x1a];
        short nextIdx = g_952858[cur * 0x1a];
        int pct;
        if (g_9ab892[faction * 0x34] == 0xc) {
            pct = 1;
        } else {
            unsigned char raw = g_9ab88f[faction * 0x34];
            int clamped = raw;
            if (clamped < 1) {
                clamped = 1;
            } else if (clamped > 0x64) {
                clamped = 0x64;
            }
            pct = clamped * 10;
        }
        int morale = pct - g_952838[record];
        if (morale < 0 || (morale <= 0x270f && morale <= 0) ||
            g_952836[record] == 0) {
            if (battle_kill(cur, a2, a3, a4, killerId, a6) != 0) {
                if (killerId > cur) {
                    killerId = killerId - 1;
                }
                if (nextIdx > cur) {
                    nextIdx = nextIdx - 1;
                }
            }
        }
        cur = nextIdx;
        if (nextIdx < 0) {
            break;
        }
    }
}
