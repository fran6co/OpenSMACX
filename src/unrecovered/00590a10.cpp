// ORIGINAL: 0x00590A10 FILE
// working copy - scaffold materialised by --work
// name      ?say_society@@YAXPAXHH@Z
// size      131 bytes
// spans     0x00590A10-0x00590A93
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645470

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_society@@YAXPAXHH@Z  at 0x00590A10  (131 bytes)
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

extern "C" char *strcat(char *, const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_0094b000 = (int *)0x0094B000;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;
void __cdecl say_society(void * a1, int a2, int a3) {
    Strings *strs = reinterpret_cast<Strings *>(g_009b90d8);
    int idx = g_0094b000[a2 * 0x35];
    char *s1 = reinterpret_cast<char *>(strs->get(idx));
    strcat(static_cast<char *>(a1), s1);
    if (a3 != 0) {
        strcat(static_cast<char *>(a1), reinterpret_cast<char *>(g_00682820));
        strcat(static_cast<char *>(a1), reinterpret_cast<char *>(g_00682e9c));
        int base = *g_009b90f8;
        int val = *reinterpret_cast<int *>(base + 0x114);
        char *s2 = reinterpret_cast<char *>(strs->get(val));
        strcat(reinterpret_cast<char *>(g_009b86a0), s2);
        strcat(static_cast<char *>(a1), reinterpret_cast<char *>(g_00682e98));
    }
}
