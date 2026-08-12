// ORIGINAL: 0x005A6270 FILE
// name      ?proto_sort@@YAXH@Z
// size      345 bytes
// spans     0x005A6270-0x005A63C9
// prototype void (__cdecl ?proto_sort@@YAXH@Z)(int factionID)
// callers   2   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D360 0x0057D3F0 0x005B5690 0x005B9F20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005A6270
// measured tier  MISMATCH
// divergence     39
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005a6270/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?proto_sort@@YAXH@Z  at 0x005A6270  (345 bytes)
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
int __cdecl arm_val(int, int);
int __cdecl weap_val(int, int);
void __cdecl sort(int, int *, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00945b40 = (int *)0x00945B40;
static int *const g_00945b44 = (int *)0x00945B44;

// Indexed table bases: each is reached via a register-computed byte
// offset (record index * 0x34, or byte-value * 0x90), so the address
// itself does work under /O2 - each needs `extern T name[]`.
extern unsigned char g_arr_9ab88c[];
extern unsigned char g_arr_9ab88e[];
extern unsigned char g_arr_9ab892[];
extern unsigned char g_arr_9ab898[];
extern unsigned char g_arr_9ab89a[];
extern unsigned char g_arr_94a379[];
extern unsigned char g_arr_94a378[];
extern int g_arr_945b44[];

void __cdecl proto_sort(int a1) {
    int sortkeys[128];

    *g_00945b40 = 0;
    int i = 0;
    do {
        int base = (i >= 0x40) ? (a1 << 6) : 0;
        int protoId = (i % 64) + base;

        g_arr_945b44[i] = protoId;

        int off = protoId * 0x34;
        int key;

        if (!(g_arr_9ab898[off] & 1) ||
            (protoId < 0x40 &&
             !has_tech(*reinterpret_cast<short *>(g_arr_9ab89a + off), a1))) {
            if (protoId < 0x40) {
                key = 999999999;
            } else {
                key = 0x34fb5e38;
                (*g_00945b40)++;
            }
        } else {
            unsigned char fieldA = g_arr_9ab88c[off];
            int t1 = g_arr_94a379[fieldA * 0x90];
            key = (t1 + g_arr_9ab892[off] * 16) << 18;
            int w = weap_val(protoId, a1);
            key += (32 - w) << 9;
            fieldA = g_arr_9ab88c[off];
            int t2 = g_arr_94a378[fieldA * 0x90];
            key += (8 - t2) << 6;
            unsigned char fieldC = g_arr_9ab88e[off];
            int ar = arm_val(fieldC, a1);
            key += 8 - ar;
            (*g_00945b40)++;
        }

        sortkeys[i] = key;
        i++;
    } while (i < 0x80);

    sort(0x80, g_00945b44, sortkeys);
}
