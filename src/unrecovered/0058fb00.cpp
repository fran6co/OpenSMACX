// ORIGINAL: 0x0058FB00 ?get_pact_hood@@YAPADHH@Z 0x0058FB00-0x0058FB61 FILE
// TRIED: short-circuit || skipping a2's lookup on a1's failing, matching the disassembly's early je; g_009b90f8 held as int** (a slot)
// working copy - scaffold materialised by --work
// size      97 bytes
// prototype 
// callers   10   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_pact_hood@@YAPADHH@Z  at 0x0058FB00  (97 bytes)
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
static int **const g_009b90f8_tbl = (int **)0x009B90F8;

char * __cdecl get_pact_hood(int a1, int a2) {
    *g_009bbfec = 0;
    *g_009bbff0 = 0;
    int index;
    if (g_00946a50[a1 * 0x167] == 0 || g_00946a50[a2 * 0x167] == 0) {
        index = 0xcd;
    } else {
        index = 0xce;
    }
    int stringId = (*g_009b90f8_tbl)[index];
    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    return (char *)strings->get(stringId);
}
