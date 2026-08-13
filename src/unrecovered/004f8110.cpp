// ORIGINAL: 0x004F8110 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?base_claimed@@YAHHHHHHH@Z
// size      133 bytes
// spans     0x004F8110-0x004F8195
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_claimed@@YAHHHHHHH@Z  at 0x004F8110  (133 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090ea44 = (int *)0x0090EA44;
static int *const g_0090ea48 = (int *)0x0090EA48;
static int *const g_0090ea4c = (int *)0x0090EA4C;
static int *const g_0090ea50 = (int *)0x0090EA50;
static int *const g_0090ea54 = (int *)0x0090EA54;
static int *const g_0090ea58 = (int *)0x0090EA58;
static int *const g_0090ea5c = (int *)0x0090EA5C;
static int *const g_0090ea60 = (int *)0x0090EA60;
int __cdecl base_claimed(int a1, int a2, int a3, int a4, int a5, int a6) {
    if (a3 == 0 && a1 == *g_0090ea44 && a2 >= *g_0090ea54) {
        return 1;
    }
    if (a4 == 0 && a1 == *g_0090ea48 && a2 >= *g_0090ea58) {
        return 1;
    }
    if (a5 == 0 && a1 == *g_0090ea4c && a2 >= *g_0090ea5c) {
        return 1;
    }
    if (a6 == 0 && a1 == *g_0090ea50 && a2 >= *g_0090ea60) {
        return 1;
    }
    return 0;
}
