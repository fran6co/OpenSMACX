// ORIGINAL: 0x00591E90 sub_591e90 0x00591E90-0x00591EF9 FILE
// working copy - scaffold materialised by --work
// size      105 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_591e90  at 0x00591E90  (105 bytes)
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
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
extern "C" int __cdecl sub_591e90(int param_1, int param_2, int param_3) {
    if (param_3 < 1) {
        return -1;
    }
    unsigned char *base = (unsigned char *)*g_0094a30c;
    int idx = *g_0068faf0 * param_2 + (param_1 >> 1);
    unsigned char *rec = base + idx * 0x2c;
    if ((rec[0] & 0xe0) < 0x60) {
        return -1;
    }
    unsigned int flags3 = rec[5] & 7;
    if (flags3 == 0) {
        return -1;
    }
    if ((int)flags3 == param_3) {
        return -1;
    }
    unsigned int *table = (unsigned int *)((char *)g_0096c9f8 + param_3 * 0x20cc + flags3 * 4);
    if (*table & 1) {
        return -1;
    }
    if ((*table & 2) == 0) {
        return -1;
    }
    return (int)flags3;
}
