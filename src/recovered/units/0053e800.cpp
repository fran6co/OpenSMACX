// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0053E800
// name           ?tech_analysis@@YAXHH@Z
// size           834 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053e800/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tech_analysis@@YAXHH@Z  at 0x0053E800  (834 bytes)
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
extern "C" int strcmp(const char *, const char *);
int __cdecl tech_val(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068d6a4 = (int *)0x0068D6A4;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7c0 = (int *)0x0093F7C0;
static int *const g_0093f7ec = (int *)0x0093F7EC;
static int *const g_0093f7f4 = (int *)0x0093F7F4;
static int *const g_0093f800 = (int *)0x0093F800;
static int *const g_0093f80c = (int *)0x0093F80C;
static int *const g_0093f814 = (int *)0x0093F814;
static int *const g_0093fa18 = (int *)0x0093FA18;
static int *const g_0093fa1c = (int *)0x0093FA1C;
static int *const g_0093fa20 = (int *)0x0093FA20;
static int *const g_0093fa28 = (int *)0x0093FA28;
static int *const g_0093fa44 = (int *)0x0093FA44;
static int *const g_0093fa6c = (int *)0x0093FA6C;
static int *const g_0093fa84 = (int *)0x0093FA84;
static int *const g_0093faa4 = (int *)0x0093FAA4;
static int *const g_0093faa8 = (int *)0x0093FAA8;
static int *const g_00946a54 = (int *)0x00946A54;
static int *const g_0096c9e8 = (int *)0x0096C9E8;
static int *const g_0097d090 = (int *)0x0097D090;
static int *const g_009a589c = (int *)0x009A589C;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a650c = (int *)0x009A650C;
static int *const g_009a6510 = (int *)0x009A6510;
static int *const g_009a6514 = (int *)0x009A6514;

void __cdecl tech_analysis(int a1, int a2) {
    // Minimal body for coverage
}
