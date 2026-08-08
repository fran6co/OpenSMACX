// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00476DA0
// name           ?mon_secrets_of_tech@@YAXH@Z
// size           317 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00476da0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_secrets_of_tech@@YAXH@Z  at 0x00476DA0  (317 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl game_year(int);
void __cdecl monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0094ca88 = (int *)0x0094CA88;
static int *const g_0094ca8c = (int *)0x0094CA8C;
static int *const g_0094f228 = (int *)0x0094F228;
static int *const g_0094f22c = (int *)0x0094F22C;
static int *const g_009a64d4 = (int *)0x009A64D4;

// Same record family as mon_facility_built: one record every 0x4F4 bytes,
// indexed by a1. A byte pointer, not a struct array or an int array -
// either lets /O2 fold the *4 element scale into the SIB addressing mode,
// dropping the original's explicit `shl`.
extern uint8_t g_records_bytes[];
extern uint8_t g_946f58_tbl[];

#define REC_YEAR        0x0    // 0x0094CA78
#define REC_FIELD1      0x4    // 0x0094CA7C
#define REC_FIELD2      0x8    // 0x0094CA80
#define REC_OWNER       0xC    // 0x0094CA84
#define REC_BUILT_FLAG  0x10   // 0x0094CA88
#define REC_OTHER_COUNT 0x14   // 0x0094CA8C
#define REC_STATUS      0x428  // 0x0094CEA0
#define REC_STRIDE      0x4f4

void __cdecl mon_secrets_of_tech(int a1) {
    int32_t year = game_year(*g_009a64d4);
    uint8_t *rec = g_records_bytes + a1 * REC_STRIDE;

    if ((g_946f58_tbl[a1 * 0x59c] & 0x80) == 0) {
        if (*(int32_t *)(rec + REC_BUILT_FLAG) == 0) {
            *(int32_t *)(rec + REC_OTHER_COUNT) = 0;
            int32_t i = 0;
            int32_t *p = (int32_t *)(g_records_bytes + REC_BUILT_FLAG);
            do {
                if (i != a1 && *p != 0) {
                    (*(int32_t *)(rec + REC_OTHER_COUNT))++;
                }
                p = (int32_t *)((char *)p + REC_STRIDE);
                i++;
            } while ((int32_t)p < (int32_t)g_0094f228);

            int32_t builder;
            if (*(int32_t *)(rec + REC_OTHER_COUNT) == 0) {
                *(int32_t *)(rec + REC_OTHER_COUNT) = 1;
                builder = a1;
            } else {
                *(int32_t *)(rec + REC_OTHER_COUNT) = 0;
                builder = a1;
                int32_t j = 0;
                int32_t *q = (int32_t *)(g_records_bytes + REC_OTHER_COUNT);
                do {
                    if (j != a1 && q[-1] != 0 && *q != 0) {
                        builder = j;
                    }
                    q = (int32_t *)((char *)q + REC_STRIDE);
                    j++;
                } while ((int32_t)q < (int32_t)g_0094f22c);
            }

            *(int32_t *)(rec + REC_OWNER) = builder;
            *(int32_t *)(rec + REC_YEAR) = year;
            *(int32_t *)(rec + REC_BUILT_FLAG) = 1;
            *(int32_t *)(rec + REC_FIELD2) = -1;
            *(int32_t *)(rec + REC_FIELD1) = -1;
        } else if (*(int32_t *)(rec + REC_FIELD2) == -1) {
            *(int32_t *)(rec + REC_FIELD2) = year;
        } else if (*(int32_t *)(rec + REC_FIELD1) == -1) {
            *(int32_t *)(rec + REC_FIELD1) = year;
        }

        *(int32_t *)(rec + REC_STATUS) = 2;
        if (a1 == *g_00939284) {
            monument(1);
        }
    }
}
