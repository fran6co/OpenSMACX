// ORIGINAL: 0x005ADE80 FILE
// working copy - scaffold materialised by --work
// name      ?replay_base@@YAXHHHH@Z
// size      131 bytes
// spans     0x005ADE80-0x005ADF03
// prototype 
// callers   5   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006262F0 0x00645930

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?replay_base@@YAXHHHH@Z  at 0x005ADE80  (131 bytes)
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
extern "C" void *memcpy(void *, const void *, unsigned int);
void log_say(char *, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00691444 = (int *)0x00691444;
static int *const g_00691450 = (int *)0x00691450;
static int *const g_009469f8 = (int *)0x009469F8;
static int *const g_009a64d4 = (int *)0x009A64D4;
struct ReplayRec8 {
    unsigned char f0;
    unsigned char f1;
    unsigned short f2;
    unsigned short f3;
    unsigned short f4;
};

void __cdecl replay_base(int a1, int a2, int a3, int a4) {
    log_say(reinterpret_cast<char *>(g_00691444), a2, a3, a4);
    log_say(reinterpret_cast<char *>(g_00691450), *g_009a64d4, a1, *g_009469f8);

    ReplayRec8 rec;
    rec.f2 = static_cast<unsigned short>(*g_009a64d4);
    rec.f0 = static_cast<unsigned char>(a1);
    rec.f3 = static_cast<unsigned short>(a2);
    rec.f4 = static_cast<unsigned short>(a3);
    rec.f1 = static_cast<unsigned char>(a4);

    char *dest = reinterpret_cast<char *>(0x950324 + *g_009469f8);
    memcpy(dest, &rec, 8);
    *g_009469f8 = *g_009469f8 + 8;
}
