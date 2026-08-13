// ORIGINAL: 0x00626D90 FILE
// working copy - scaffold materialised by --work
// name      ?scan_out_line_poly_dest@@YAIHH@Z
// size      123 bytes
// spans     0x00626D90-0x00626E0B
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?scan_out_line_poly_dest@@YAIHH@Z  at 0x00626D90  (123 bytes)
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
static int *const g_009bc00c = (int *)0x009BC00C;
static int *const g_009bc01c = (int *)0x009BC01C;
static int *const g_009bc030 = (int *)0x009BC030;
static int *const g_009bc03c = (int *)0x009BC03C;
static int *const g_009bc040 = (int *)0x009BC040;
static int *const g_009bc044 = (int *)0x009BC044;
// No explicit `mov eax` before the final `ret` - the original never
// produces a value, so this returns void rather than the contract's
// nominal unsigned int.
void __cdecl scan_out_line_poly_dest(int a1, int a2) {
    int addend = *g_009bc030;
    if (a2 <= *g_009bc01c) return;
    if (a1 >= *g_009bc044) return;
    if (a1 >= a2) return;
    if (*g_009bc044 <= a2) a2 = *g_009bc044;
    if (a1 < *g_009bc01c) a1 = *g_009bc01c;
    int count = a2 - a1;
    if (count == 0) return;
    if (a1 > a2) return;
    unsigned char *dst = reinterpret_cast<unsigned char *>(*g_009bc03c * *g_009bc040 + a1 + *g_009bc00c);
    do {
        *dst = *reinterpret_cast<unsigned char *>(*dst + addend);
        dst++;
        count--;
    } while (count != 0);
}
