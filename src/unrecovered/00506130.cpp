// ORIGINAL: 0x00506130 ?battle_kill_stack@@YAXHPAHPAHPAHHH@Z 0x00506130-0x005061AE FILE BYTE_EXACT
// symbol    ?battle_kill_stack@@YAXHPAH00HH@Z
// working copy - scaffold materialised by --work
// size      126 bytes
// prototype void (__cdecl ?battle_kill_stack@@YAXHPAHPAHPAHHH@Z)(int vehID1, int*, int*, int*, int vehID2, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00505D80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?battle_kill_stack@@YAXHPAHPAHPAHHH@Z  at 0x00506130  (126 bytes)
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
int battle_kill(int, int *, int *, int *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00952858 = (int *)0x00952858;
static int *const g_0095285a = (int *)0x0095285A;
void __cdecl battle_kill_stack(int a1, int * a2, int * a3, int * a4, int a5, int a6) {
    if (a1 < 0) {
        return;
    }
    short *table1 = reinterpret_cast<short *>(g_0095285a);
    short next = table1[a1 * 0x1a];
    while (next >= 0) {
        a1 = next;
        next = table1[a1 * 0x1a];
    }
    short *table2 = reinterpret_cast<short *>(g_00952858);
    while (a1 >= 0) {
        int nextId = table2[a1 * 0x1a];
        int killed = battle_kill(a1, a2, a3, a4, a5, a6);
        if (killed != 0) {
            if (a5 > a1) {
                a5--;
            }
            if (nextId > a1) {
                nextId--;
            }
        }
        a1 = nextId;
    }
}
