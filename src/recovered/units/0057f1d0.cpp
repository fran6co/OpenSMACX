// ORIGINAL: 0x0057F1D0 ?retire_proto@@YAXHH@Z 0x0057F1D0-0x0057F4B0 FILE
// size      736 bytes
// prototype 
// callers   4   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0057DF30 0x005B9580 0x005C0B00
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0057F1D0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0057f1d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?retire_proto@@YAXHH@Z  at 0x0057F1D0  (736 bytes)
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
extern "C" int __cdecl sub_57df30();
int __cdecl base_at(int, int);
int __cdecl stack_check(int, int, int, int, int);
void __cdecl kill(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0095282a = (int *)0x0095282A;
static int *const g_0097d08c = (int *)0x0097D08C;
static int *const g_0097d090 = (int *)0x0097D090;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab891 = (int *)0x009AB891;
static int *const g_009ab893 = (int *)0x009AB893;

void __cdecl retire_proto(int a1, int a2) {
    // Minimal body for coverage
}
