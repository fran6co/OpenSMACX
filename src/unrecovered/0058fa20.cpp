// ORIGINAL: 0x0058FA20 FILE
// RULED-OUT: `extern int g_00946a50[];` for the indexed-table-base global
//            conflicts (C2373) with the scaffold's own fixed-pointer decl of
//            the same name; used the scaffold's `g_00946a50[idx]` directly
//            instead, which compiles but diverges at #3 (xor vs mov)
// working copy - scaffold materialised by --work
// name      ?get_pacts@@YAPADH@Z
// size      73 bytes
// spans     0x0058FA20-0x0058FA69
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_pacts@@YAPADH@Z  at 0x0058FA20  (73 bytes)
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

typedef int BOOL;
class Heap;
typedef void * LPVOID;
class Strings;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
};

class Strings : public Heap { public:
    BOOL is_populated_;
    int get(int);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00946a50 = (int *)0x00946A50;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;
char * __cdecl get_pacts(int a1) {
    *g_009bbff0 = 1;
    int v = g_00946a50[a1 * 359];
    *g_009bbfec = v;
    int idx = (v != 0) + 0xcb;
    int *table = *(int **)g_009b90f8;
    return (char *)((Strings *)g_009b90d8)->get(table[idx]);
}
