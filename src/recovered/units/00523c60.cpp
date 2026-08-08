// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00523C60
// name           ?set_time_controls@@YAXXZ
// size           361 bytes
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00523c60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_time_controls@@YAXXZ  at 0x00523C60  (361 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_0093e968 = (int *)0x0093E968;
static int *const g_0097f094 = (int *)0x0097F094;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e0 = (int *)0x009A64E0;
static int *const g_009a64e4 = (int *)0x009A64E4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009b2070 = (int *)0x009B2070;
// Table bases the loop walks by incrementing the address itself
// (`add edi,0x20cc` / `add edx,0x34`) - a folded pointer constant loses
// that addressing mode, so these are extern arrays instead.
extern int32_t g_0095282c[];
extern int32_t g_00970b00[];
// Per-difficulty (or per-scenario) parameter table, indexed by
// DAT_009a64e0 * 8 (int units, i.e. 32 bytes/record). The emitter does
// not surface base-immediate/offset-register globals on its own, so
// these six fields are declared explicitly rather than folded.
extern int32_t g_0094f1bc[];
extern int32_t g_0094f1c0[];
extern int32_t g_0094f1c4[];
extern int32_t g_0094f1c8[];
extern int32_t g_0094f1d0[];
extern int32_t g_0094f1d4[];

void __cdecl set_time_controls() {
    int32_t idx = *g_009a64e0;
    int32_t activeCount = 0;
    int32_t idx8 = idx * 8;
    int32_t *pFaction = g_00970b00;
    int32_t supportSum = 0;
    int32_t bestTime = g_0094f1bc[idx8];
    int32_t weightedSum = 0;
    uint32_t flagsByte = *g_009a64e8 & 0xff;
    int32_t bit = 1;

    do {
        if (*g_0093a938 != 0 && *g_009b2070 == 0) {
            int32_t divisor = g_0094f1d0[idx8];
            if (divisor != 0 && *g_009a64d4 % divisor == 0) {
                int32_t next = *pFaction + 1;
                int32_t limit = g_0094f1d4[idx8];
                if (next >= limit) {
                    next = limit;
                }
                *pFaction = next;
            }
        }

        if (flagsByte & (1u << bit)) {
            weightedSum += g_0094f1c0[idx8] * pFaction[-0x3fe];
            ++activeCount;
            int32_t vehCount = *g_009a64c8;
            if (vehCount > 0) {
                int32_t *pVeh = g_0095282c;
                int32_t n = vehCount;
                do {
                    uint8_t *rec = reinterpret_cast<uint8_t *>(pVeh);
                    if (rec[0xa] == bit) {
                        uint8_t order = rec[0xd];
                        if (order != 1 && order != 2) {
                            int32_t flags = *pVeh;
                            if ((flags & 0x200) == 0 &&
                                (flags & 0x1000200) != 0x1000200 &&
                                (flags & 0x4000) == 0) {
                                supportSum += g_0094f1c4[idx8];
                            }
                        }
                    }
                    pVeh += 13;
                    --n;
                } while (n != 0);
            }
        }

        pFaction += 0x833;
        ++bit;
    } while (pFaction < reinterpret_cast<int32_t *>(g_0097f094));

    if (activeCount != 0) {
        int32_t candidate = (supportSum + weightedSum) / activeCount +
                             g_0094f1c8[idx8] * (*g_0093e968);
        if (bestTime <= candidate) {
            bestTime = candidate;
        }
    }
    if (*g_009a64d4 < 6) {
        bestTime = bestTime * 2;
    }
    *g_009a64e4 = bestTime * 1000;
}
