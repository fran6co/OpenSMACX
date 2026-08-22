// ORIGINAL: 0x00477A60 ?mon_killed_faction@@YAXHH@Z 0x00477A60-0x00477B0F FILE
// TRIED: g_00946f58 already scaffold-declared as `int *const` (plain load elsewhere) -> C2373 redefinition; renamed the address-does-work extern to g_byteflags_946f58 alongside three new `extern int name[]` table bases (0094cd60/ca08/cea0), masked relocations, cheap per the documented lever. MISMATCH 0.86 sim: original spills `year` to an extra `push ecx` frame slot our register allocation never needed; not chased further.
// working copy - scaffold materialised by --work
// size      175 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00476A50 0x005C89A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?mon_killed_faction@@YAXHH@Z  at 0x00477A60  (175 bytes)
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
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a64d4 = (int *)0x009A64D4;
extern int g_0094cd60[];
extern int g_0094ca08[];
extern int g_0094cea0[];
extern unsigned char g_byteflags_946f58[];

void __cdecl mon_killed_faction(int a1, int a2) {
    int year = game_year(*g_009a64d4);
    int chosen = -1;
    char *slotBase = reinterpret_cast<char *>(g_0094cd60) + a1 * 0x4F4;
    for (int i = 0; i < 6; i++) {
        if (*reinterpret_cast<int *>(slotBase) == 0 && chosen == -1) {
            chosen = i + 0xF;
        }
        slotBase += 0x38;
    }
    if (chosen != -1) {
        char *rec = reinterpret_cast<char *>(g_0094ca08) + a1 * 0x4F4 + chosen * 0x38;
        *reinterpret_cast<int *>(rec) = year;
        *reinterpret_cast<int *>(rec + 8) = a2;
        *reinterpret_cast<int *>(rec + 0x14) = 1;
        *reinterpret_cast<int *>(rec + 0x10) = 1;
        *reinterpret_cast<int *>(rec + 0xC) = a1;
        *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0094cea0) + a1 * 0x4F4) = chosen;
        if ((*reinterpret_cast<unsigned char *>(g_byteflags_946f58 + a1 * 0x59C) & 0x80) == 0 &&
            a1 == *g_00939284) {
            monument(1);
        }
    }
}
