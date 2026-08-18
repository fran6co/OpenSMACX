// ORIGINAL: 0x0063AD60 sub_63ad60 0x0063AD60-0x0063AE12 FILE
// working copy - scaffold materialised by --work
// size      178 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63ad60  at 0x0063AD60  (178 bytes)
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
static int *const g_0063aee0 = (int *)0x0063AEE0;
static int *const g_0063af00 = (int *)0x0063AF00;
static int *const g_0063af20 = (int *)0x0063AF20;
static int *const g_0063af40 = (int *)0x0063AF40;
static int *const g_00664000 = (int *)0x00664000;
static int *const g_006647f0 = (int *)0x006647F0;
static int *const g_00664e50 = (int *)0x00664E50;
static int *const g_00665640 = (int *)0x00665640;
static int *const g_00665ca0 = (int *)0x00665CA0;
static int *const g_00666440 = (int *)0x00666440;
static int *const g_00666a80 = (int *)0x00666A80;
static int *const g_00666fd0 = (int *)0x00666FD0;
static int *const g_00667380 = (int *)0x00667380;
static int *const g_00667b20 = (int *)0x00667B20;
extern "C" void __cdecl sub_63ad60(int a1) {
    unsigned char *obj = reinterpret_cast<unsigned char *>(a1);
    int typ = *reinterpret_cast<unsigned char *>(*reinterpret_cast<int *>(obj + 4) + 0x20);
    if (typ == 1) {
        if (*obj & 1) {
            *reinterpret_cast<int **>(obj + 0x6c) = g_00665640;
            *reinterpret_cast<int **>(obj + 0x70) = g_00664e50;
            *reinterpret_cast<int **>(obj + 0x7c) = g_0063af40;
            *reinterpret_cast<int **>(obj + 0x74) = g_00666fd0;
            *reinterpret_cast<int **>(obj + 0x78) = g_00666a80;
            return;
        }
        *reinterpret_cast<int **>(obj + 0x6c) = g_006647f0;
        *reinterpret_cast<int **>(obj + 0x70) = g_00664000;
        *reinterpret_cast<int **>(obj + 0x7c) = g_0063af00;
    } else if (typ == 2) {
        if (*obj & 1) {
            *reinterpret_cast<int **>(obj + 0x6c) = g_00667b20;
            *reinterpret_cast<int **>(obj + 0x70) = g_00667380;
            *reinterpret_cast<int **>(obj + 0x7c) = g_0063af20;
            *reinterpret_cast<int **>(obj + 0x74) = g_00666fd0;
            *reinterpret_cast<int **>(obj + 0x78) = g_00666a80;
            return;
        }
        *reinterpret_cast<int **>(obj + 0x6c) = g_00666440;
        *reinterpret_cast<int **>(obj + 0x70) = g_00665ca0;
        *reinterpret_cast<int **>(obj + 0x7c) = g_0063aee0;
        *reinterpret_cast<int **>(obj + 0x74) = g_00666fd0;
        *reinterpret_cast<int **>(obj + 0x78) = g_00666a80;
        return;
    }
    *reinterpret_cast<int **>(obj + 0x74) = g_00666fd0;
    *reinterpret_cast<int **>(obj + 0x78) = g_00666a80;
}
