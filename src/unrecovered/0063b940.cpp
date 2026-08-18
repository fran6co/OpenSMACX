// ORIGINAL: 0x0063B940 ?trig_init@@YAHXZ 0x0063B940-0x0063B9AE FILE
// working copy - scaffold materialised by --work
// size      110 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0063B9B0 0x006458C8 0x006463E4

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?trig_init@@YAHXZ  at 0x0063B940  (110 bytes)
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
extern "C" int __cdecl _ftol();
extern "C" int __cdecl sin();
int sin(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00671028 = (int *)0x00671028;
static int *const g_00671030 = (int *)0x00671030;
static int *const g_009be6dc = (int *)0x009BE6DC;
static int *const g_009beadc = (int *)0x009BEADC;
extern "C" double __cdecl trig_sin_impl(double);

int __cdecl trig_init() {
    int *table = reinterpret_cast<int *>(g_009be6dc);
    int *table_end = reinterpret_cast<int *>(g_009beadc);
    double k1 = *reinterpret_cast<double *>(g_00671030);
    double k2 = *reinterpret_cast<double *>(g_00671028);
    int i = 0;
    do {
        table[i] = static_cast<int>(trig_sin_impl(static_cast<double>(i) * k1) * k2);
        i = i + 1;
    } while (table + i < table_end);

    sin(0x40000000, 100);
    return sin(0xe02d82d7, 100);
}
