// ORIGINAL: 0x00506A60 FILE
// working copy - scaffold materialised by --work
// name      ?battle_fight@@YAHHHHHPAH@Z
// size      139 bytes
// spans     0x00506A60-0x00506AEB
// prototype int (__cdecl ?battle_fight@@YAHHHHHPAH@Z)(int vehID, int, int, int, int*)
// callers   6   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00506AF0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?battle_fight@@YAHHHHHPAH@Z  at 0x00506A60  (139 bytes)
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
int battle_fight(int, int, int, int, int, int, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066ef50 = (int *)0x0066EF50;
static int *const g_0066ef74 = (int *)0x0066EF74;
static int *const g_0066efbc = (int *)0x0066EFBC;
static int *const g_0066f440 = (int *)0x0066F440;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_0094988c = (int *)0x0094988C;
extern short g_00952828[];

int __cdecl battle_fight(int a1, int a2, int a3, int a4, int * a5) {
    int base;
    if (a3 != 0) {
        base = g_0066efbc[a2];
    } else {
        base = g_0066ef50[a2];
    }
    int val1 = g_00952828[a1 * 26] + base;

    if ((*reinterpret_cast<uint8_t *>(g_0094988c) & 1) == 0) {
        int mod = *g_00949870;
        if (val1 < 0) {
            val1 += mod;
        } else if (val1 >= mod) {
            val1 -= mod;
        }
    }

    int base2;
    if (a3 != 0) {
        base2 = g_0066f440[a2];
    } else {
        base2 = g_0066ef74[a2];
    }
    int val2 = g_00952828[a1 * 26 + 1] + base2;

    return battle_fight(a1, a2, val1, val2, a3, a4, a5);
}
