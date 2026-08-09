// ORIGINAL: 0x005A63D0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005A63D0
// name           ?proto_sort_2@@YAXH@Z
// size           315 bytes
// measured tier  MISMATCH
// divergence     46
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005a63d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?proto_sort_2@@YAXH@Z  at 0x005A63D0  (315 bytes)
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
bool __cdecl has_tech(int, int);
void __cdecl sort(int, int *, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00945b40 = (int *)0x00945B40;
static int *const g_00945b44 = (int *)0x00945B44;

extern int g_00945b40_ctr;
extern int g_00945b44_arr[];
extern uint8_t g_009ab898[];
extern uint8_t g_009ab88d[];
extern uint8_t g_009ab894[];
extern uint8_t g_009ab89a[];
extern int8_t g_0094ae68[];
extern uint8_t g_0096d238[];

void __cdecl proto_sort_2(int a1) {
    g_00945b40_ctr = 0;
    int arr[128];

    for (int i = 0; i < 128; i++) {
        int base = (i >= 0x40) ? (a1 << 6) : 0;
        int m = i % 64;
        int rec = m + base;
        g_00945b44_arr[i] = rec;

        int off = rec * 0x34;

        if ((g_009ab898[off] & 1) == 0) {
            goto skip;
        }
        if (rec < 0x40) {
            int cat = g_009ab88d[off];
            if (g_0094ae68[cat * 16] < 0 || rec == 0xF ||
                !has_tech(*reinterpret_cast<short *>(g_009ab89a + off), a1)) {
                goto skip;
            }
        }

        if ((g_009ab894[off] & (1 << a1)) == 0) {
            goto af;
        }
        if (rec < 0x40) {
            goto a9;
        }
        {
            uint8_t v = g_0096d238[a1 * 0x20CC + rec];
            rec += (v != 0) ? 0x800 : 0x1000;
            ++g_00945b40_ctr;
            goto store;
        }
    a9:
        rec += 0x2000;
    af:
        ++g_00945b40_ctr;
        goto store;

    skip:
        if (rec < 0x40) {
            rec = 999999999;
        } else {
            rec += 0x400;
            ++g_00945b40_ctr;
        }

    store:
        arr[i] = rec;
    }

    sort(0x80, g_00945b44_arr, arr);
}
