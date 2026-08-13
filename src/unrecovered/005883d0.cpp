// ORIGINAL: 0x005883D0 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?scroll_small@@YAXXZ
// size      141 bytes
// spans     0x005883D0-0x0058845D
// prototype 
// callers   6   call targets   0
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?scroll_small@@YAXXZ  at 0x005883D0  (141 bytes)
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
static int *const g_007796b0 = (int *)0x007796B0;
static int *const g_0078de30 = (int *)0x0078DE30;
static int *const g_0078de5c = (int *)0x0078DE5C;
static int *const g_0078de88 = (int *)0x0078DE88;
static int *const g_0078deb4 = (int *)0x0078DEB4;
static int *const g_0078dee0 = (int *)0x0078DEE0;
static int *const g_0078df0c = (int *)0x0078DF0C;
static int *const g_0078df38 = (int *)0x0078DF38;
static int *const g_0078df64 = (int *)0x0078DF64;
static int *const g_0078df90 = (int *)0x0078DF90;
static int *const g_0078dfbc = (int *)0x0078DFBC;
static int *const g_0078dfe8 = (int *)0x0078DFE8;
static int *const g_0078e014 = (int *)0x0078E014;
static int *const g_009b8dd4 = (int *)0x009B8DD4;
static int *const g_009b8de8 = (int *)0x009B8DE8;
static int *const g_009b8dec = (int *)0x009B8DEC;
static int *const g_009b8df0 = (int *)0x009B8DF0;
static int *const g_009b8df4 = (int *)0x009B8DF4;
static int *const g_009b8df8 = (int *)0x009B8DF8;
static int *const g_009b8dfc = (int *)0x009B8DFC;
static int *const g_009b8e00 = (int *)0x009B8E00;
static int *const g_009b8e04 = (int *)0x009B8E04;
static int *const g_009b8e08 = (int *)0x009B8E08;
static int *const g_009b8e0c = (int *)0x009B8E0C;
static int *const g_009b8e10 = (int *)0x009B8E10;
static int *const g_009b8e14 = (int *)0x009B8E14;
static int *const g_009b8e1c = (int *)0x009B8E1C;
void __cdecl scroll_small() {
    *g_009b8df4 = 0x78de30;
    *g_009b8df8 = 0x78de5c;
    *g_009b8dfc = 0x78de88;
    *g_009b8e0c = 0x78deb4;
    *g_009b8e10 = 0x78dee0;
    *g_009b8e14 = 0x78df0c;
    *g_009b8de8 = 0x78df38;
    *g_009b8dec = 0x78df64;
    *g_009b8df0 = 0x78df90;
    *g_009b8e00 = 0x78dfbc;
    *g_009b8e04 = 0x78dfe8;
    *g_009b8e08 = 0x78e014;
    *g_009b8e1c = 0x7796b0;
    *g_009b8dd4 = 0xc;
}
