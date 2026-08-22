// ORIGINAL: 0x0058F610 ?say_special@@YAXPADPADH@Z 0x0058F610-0x0058F6F6 FILE
// TRIED: `extern int g_946a50_table[]` (indexed by `a3 * 0x167`, a genuine table lookup) instead of the given `static int *const` pointer; string-literal globals are passed as the ADDRESS itself (`reinterpret_cast<char*>(g_ADDR)`, not `*g_ADDR`) since they are `char*` arguments to strcat, not dereferenced ints; the 0/1/2 dispatch is a plain if/else-if chain, each branch with its own strcat call (matching ghidra's `goto`-past-the-shared-call shape semantically, though not byte-for-byte).
// TRIED: original computes `a3*0x167` via a lea/lea/shl/sub strength-reduction chain (avoiding imul); both `table[a3*0x167]` and a separately-named `idx = a3*0x167` local compiled to an `imul`, no difference between the two forms.
// working copy - scaffold materialised by --work
// size      230 bytes
// prototype 
// callers   4   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00625880 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_special@@YAXPADPADH@Z  at 0x0058F610  (230 bytes)
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
extern "C" char *strcat(char *, const char *);
int parse_string(char *, char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068fa90 = (int *)0x0068FA90;
static int *const g_0068fa94 = (int *)0x0068FA94;
static int *const g_0068fa98 = (int *)0x0068FA98;
static int *const g_0068fa9c = (int *)0x0068FA9C;
static int *const g_0068faa0 = (int *)0x0068FAA0;
static int *const g_0068faa8 = (int *)0x0068FAA8;
static int *const g_0068faac = (int *)0x0068FAAC;
static int *const g_00946a50 = (int *)0x00946A50;
extern int g_946a50_table[];

void __cdecl say_special(char * a1, char * a2, int a3) {
    char buf1[256];
    char buf2[256];

    buf1[0] = 0;
    int iVar1 = g_946a50_table[a3 * 0x167];
    strcat(buf1, reinterpret_cast<char *>(g_0068fa90));

    if (iVar1 == 0) {
        strcat(buf1, reinterpret_cast<char *>(g_0068fa94));
    } else if (iVar1 == 1) {
        strcat(buf1, reinterpret_cast<char *>(g_0068fa98));
    } else if (iVar1 == 2) {
        strcat(buf1, reinterpret_cast<char *>(g_0068fa9c));
    }

    strcat(buf1, reinterpret_cast<char *>(g_0068faa0));
    strcat(buf1, reinterpret_cast<char *>(g_0068faa8));
    strcat(buf1, a2);
    strcat(buf1, reinterpret_cast<char *>(g_0068faac));

    parse_string(buf1, buf2);
    strcat(a1, buf2);
}
