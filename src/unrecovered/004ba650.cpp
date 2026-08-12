// ORIGINAL: 0x004BA650 BYTE_EXACT FILE
// LEVER: test/push  0x9A64B0 as a named `extern int`, not a const-address pointer:
//   the pointer spelling let VC6 cache the flag word in eax across the table
//   read and push/pop esi for the parameter; a real global may be aliased by the
//   table pointer, so it reloads, keeps `a1` in scratch edx, and `|=` lowers to
//   load/or/store instead of `or [mem], reg`.
// working copy - scaffold materialised by --work
// name      ?tut_check2@@YAHH@Z
// size      94 bytes
// spans     0x004BA650-0x004BA6AE
// prototype 
// callers   9   call targets   0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tut_check2@@YAHH@Z  at 0x004BA650  (94 bytes)
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
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a6490 = (int *)0x009A6490;
static int *const g_009a64b0 = (int *)0x009A64B0;

// 0x009A64B0, the bits already shown by this half of the tutorial. It has to be
// a named global rather than `g_009a64b0`: the const-address spelling is what
// lets VC6 cache it across the table read, and the original reloads it.
extern int TutorialFlags2;

int __cdecl tut_check2(int a1) {
    if (*g_0093f660) {
        return 0;
    }
    if (!(*g_009a6490 & 0x20)) {
        return 0;
    }
    if (a1 & TutorialFlags2) {
        return 0;
    }
    if (g_00946f58[*g_00939284 * 359] & 0x80) {
        return 0;
    }
    TutorialFlags2 |= a1;
    return 1;
}
