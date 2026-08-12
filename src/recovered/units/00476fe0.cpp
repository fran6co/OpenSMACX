// ORIGINAL: 0x00476FE0 FILE
// name      ?mon_facility_built@@YAXHPAD@Z
// size      274 bytes
// spans     0x00476FE0-0x004770F2
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0 0x00645E90
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00476FE0
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00476fe0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_facility_built@@YAXHPAD@Z  at 0x00476FE0  (274 bytes)
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
extern "C" int __cdecl strncpy();
int __cdecl game_year(int);
void __cdecl monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0094caf8 = (int *)0x0094CAF8;
static int *const g_0094cafc = (int *)0x0094CAFC;
static int *const g_0094f298 = (int *)0x0094F298;
static int *const g_0094f29c = (int *)0x0094F29C;
static int *const g_009a64d4 = (int *)0x009A64D4;

char *__cdecl strncpy(char *dest, const char *src, unsigned int n);

// One record every 0x4F4 bytes, indexed by a1: built_flag at +0x10,
// other_count at +0x14 (immediately after built_flag - the second loop's
// `q[-1]` reaches back into built_flag), owner at +0xC, year_built at
// +0x0, status at +0x3B8, a 24-byte name buffer at +0x3E4. A byte
// pointer, not a struct array: /O2 recognises a struct-typed array whose
// element size is a multiple of 4 and folds the *4 into the SIB
// addressing mode (`[reg*4+disp]`), which is more compact than the
// original's single `a1 * 0x4F4` computed once into esi and reused
// across every field via plain displacement. Tried both ways - the
// struct form and this one - and neither reproduces the original's extra
// `shl esi, 2`; recorded as a wall below.
extern uint8_t g_records_bytes[];
extern uint8_t g_946f58_tbl[];

#define REC_YEAR_BUILT  0x0
#define REC_OWNER       0xC
#define REC_BUILT_FLAG  0x10
#define REC_OTHER_COUNT 0x14
#define REC_STATUS      0x3B8
#define REC_NAME        0x3E4
#define REC_STRIDE      0x4f4

void __cdecl mon_facility_built(int a1, char * a2) {
    int32_t year = game_year(*g_009a64d4);
    uint8_t *rec = g_records_bytes + a1 * REC_STRIDE;

    if (*(int32_t *)(rec + REC_BUILT_FLAG) == 0 &&
        (g_946f58_tbl[a1 * 0x59c] & 0x80) == 0) {
        *(int32_t *)(rec + REC_OTHER_COUNT) = 0;
        int32_t i = 0;
        int32_t *p = (int32_t *)(g_records_bytes + REC_BUILT_FLAG);
        do {
            if (i != a1 && *p != 0) {
                (*(int32_t *)(rec + REC_OTHER_COUNT))++;
            }
            p = (int32_t *)((char *)p + REC_STRIDE);
            i++;
        } while ((int32_t)p < (int32_t)g_0094f298);

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
            } while ((int32_t)q < (int32_t)g_0094f29c);
        }

        *(int32_t *)(rec + REC_OWNER) = builder;
        *(int32_t *)(rec + REC_BUILT_FLAG) = 1;
        *(int32_t *)(rec + REC_YEAR_BUILT) = year;
        strncpy((char *)(rec + REC_NAME), a2, 0x18);
        bool isLocal = (a1 == *g_00939284);
        *(int32_t *)(rec + REC_STATUS) = 4;
        if (isLocal) {
            monument(1);
        }
    }
}
