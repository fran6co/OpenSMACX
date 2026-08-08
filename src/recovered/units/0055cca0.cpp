// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0055CCA0
// name           ?pact_unpact@@YAXHH@Z
// size           373 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055cca0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pact_unpact@@YAXHH@Z  at 0x0055CCA0  (373 bytes)
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
char * __cdecl get_his_her(int, int);
char * __cdecl get_pact_hood(int, int);
int __cdecl parse_says(int, char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e604 = (int *)0x0068E604;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// The per-record tables this body walks: `esi = R * 0x59C` is a byte
// offset into a fixed table, computed once and reused at several field
// offsets within the record - the indexed-table-base lever, so each field
// is its own `extern int[]`, not a pointer constant the address itself
// could be folded out of.
extern int g_00946a50[];
extern int g_00946a84[];
extern int g_00946a9c[];
extern int g_00946d34[];
extern int g_00946d4c[];
extern int g_00946d50[];

void __cdecl pact_unpact(int a1, int a2) {
    *g_009bbff0 = 0;
    *g_009bbfec = g_00946a50[a1 * 0x167];
    parse_says(0, reinterpret_cast<char *>(&g_00946a9c[a1 * 0x167]), -1, -1);

    *g_009bbfec = g_00946a50[a1 * 0x167];
    *g_009bbff0 = 0;
    parse_says(1, reinterpret_cast<char *>(&g_00946a84[a1 * 0x167]), -1, -1);

    *g_009bbfec = g_00946d4c[a1 * 0x167];
    *g_009bbff0 = g_00946d50[a1 * 0x167];
    parse_says(2, reinterpret_cast<char *>(&g_00946d34[a1 * 0x167]), -1, -1);

    parse_says(3, get_he_she(a1, 0), -1, -1);
    parse_says(4, get_his_her(a1, 0), -1, -1);
    parse_says(5, get_pact_hood(a1, a2), -1, -1);

    *g_009bbff0 = 0;
    *g_009bbfec = g_00946a50[a2 * 0x167];
    parse_says(6, reinterpret_cast<char *>(&g_00946a9c[a2 * 0x167]), -1, -1);

    *g_009bbfec = g_00946a50[a2 * 0x167];
    *g_009bbff0 = 0;
    parse_says(7, reinterpret_cast<char *>(&g_00946a84[a2 * 0x167]), -1, -1);

    *g_009bbfec = g_00946d4c[a2 * 0x167];
    *g_009bbff0 = g_00946d50[a2 * 0x167];
    parse_says(8, reinterpret_cast<char *>(&g_00946d34[a2 * 0x167]), -1, -1);

    reinterpret_cast<NetMsg *>(g_00805338)->pop(
        reinterpret_cast<const char *>(g_0068e604), 5000, 0, 0);
}
