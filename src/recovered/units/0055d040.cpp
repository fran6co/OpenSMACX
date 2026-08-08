// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0055D040
// name           ?pact_truce@@YAXHH@Z
// size           321 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055d040/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pact_truce@@YAXHH@Z  at 0x0055D040  (321 bytes)
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

class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
char * __cdecl get_he_she(int, int);
int __cdecl parse_says(int, char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e620 = (int *)0x0068E620;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Per-faction record table, indexed by faction id - `esi` in the
// disassembly is a shared BYTE offset (`faction * 0x59C`), added as a
// register to six different field displacements within the same record.
// A pointer constant folds the literal away here (the address itself
// does work: it is the LEA/ADD base for an indexed access), so these are
// `extern int name[]`, indexed with the record stride divided by
// sizeof(int) (0x59C / 4 = 359), per the indexed-table-base lever.
extern int g_00946a50[];
extern int g_00946a84[];
extern int g_00946a9c[];
extern int g_00946d34[];
extern int g_00946d4c[];
extern int g_00946d50[];

void __cdecl pact_truce(int a1, int a2) {
    *g_009bbff0 = 0;
    int record = a1 * 359;
    *g_009bbfec = g_00946a50[record];
    parse_says(0, reinterpret_cast<char *>(&g_00946a9c[record]), -1, -1);
    *g_009bbfec = g_00946a50[record];
    *g_009bbff0 = 0;
    parse_says(1, reinterpret_cast<char *>(&g_00946a84[record]), -1, -1);
    *g_009bbfec = g_00946d4c[record];
    *g_009bbff0 = g_00946d50[record];
    parse_says(2, reinterpret_cast<char *>(&g_00946d34[record]), -1, -1);
    parse_says(3, get_he_she(a1, 0), -1, -1);
    *g_009bbff0 = 0;
    int record2 = a2 * 359;
    *g_009bbfec = g_00946a50[record2];
    parse_says(4, reinterpret_cast<char *>(&g_00946a9c[record2]), -1, -1);
    *g_009bbfec = g_00946a50[record2];
    *g_009bbff0 = 0;
    parse_says(5, reinterpret_cast<char *>(&g_00946a84[record2]), -1, -1);
    *g_009bbfec = g_00946d4c[record2];
    *g_009bbff0 = g_00946d50[record2];
    parse_says(6, reinterpret_cast<char *>(&g_00946d34[record2]), -1, -1);
    reinterpret_cast<NetMsg *>(g_00805338)->pop(
        reinterpret_cast<const char *>(g_0068e620), 5000, 0, 0);
}
