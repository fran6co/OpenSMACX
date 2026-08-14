// ORIGINAL: 0x00477840 FILE
// name      ?mon_built_preserve@@YAXH@Z
// size      254 bytes
// spans     0x00477840-0x0047793E
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0
// RULED-OUT: single `if(!(flag)&&active==0)` short-circuit form vs hoisting
//            `winner`/`count=0` before the scan loop - both land the same
//            MISMATCH #3 (original reserves the stack slot for `year` with
//            `push ecx` ahead of the ebx/esi pushes; every C form tried
//            here has the compiler push ebx/esi first instead).

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_built_preserve@@YAXH@Z  at 0x00477840  (254 bytes)
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
int game_year(int);
void monument(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_009a64d4 = (int *)0x009A64D4;

// Indexed table bases: the address itself does work (lea-chain scaling by
// the non-power-of-two record stride), so these need extern arrays rather
// than the scaffold's pointer constants above (which would let /O2 fold the
// literal address away). Relocation is masked by the comparison, so the
// numeric value these bind to does not matter.
struct MonFlags {
    unsigned char flag;
    char pad[0x59C - 1];
};
extern MonFlags g_monflags[];

struct MonRecord {
    char pad_00[0x150];
    int last_year;        // -> 0x94cca8
    char pad_154[0x15C - 0x154];
    int last_builder;     // -> 0x94ccb4
    int active;           // -> 0x94ccb8
    int count;             // -> 0x94ccbc
    char pad_168[0x348 - 0x168];
    int state;              // -> 0x94cea0
    char pad_end[0x4F4 - 0x34C];
};
extern MonRecord g_monrec[];

void __cdecl mon_built_preserve(int a1) {
    int year = game_year(*g_009a64d4);
    if (!(g_monflags[a1].flag & 0x80) && g_monrec[a1].active == 0) {
        g_monrec[a1].count = 0;
        for (int i = 0; i < 8; i++) {
            if (i != a1 && g_monrec[i].active != 0) {
                g_monrec[a1].count++;
            }
        }
        int winner = a1;
        if (g_monrec[a1].count == 0) {
            g_monrec[a1].count = 1;
        } else {
            g_monrec[a1].count = 0;
            for (int j = 0; j < 8; j++) {
                if (j != a1 && g_monrec[j].active != 0 && g_monrec[j].count != 0) {
                    winner = j;
                }
            }
        }
        g_monrec[a1].active = 1;
        g_monrec[a1].last_builder = winner;
        g_monrec[a1].last_year = year;
        g_monrec[a1].state = 0xc;
        if (a1 == *g_00939284) {
            monument(1);
        }
    }
}
