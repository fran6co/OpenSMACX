// ORIGINAL: 0x0058FA70 ?get_pacts@@YAPADHH@Z 0x0058FA70-0x0058FAFA FILE
// working copy - scaffold materialised by --work
// size      138 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_pacts@@YAPADHH@Z  at 0x0058FA70  (138 bytes)
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
char * __cdecl get_pacts(int a1, int a2) {
    int idx1 = a1 * 0x167;
    int e1 = g_00946a50[idx1];
    int e2 = 0;
    if (e1 != 0) {
        int idx2 = a2 * 0x167;
        e2 = g_00946a50[idx2];
    }
    *g_009bbfec = (e1 != 0 && e2 != 0) ? 1 : 0;
    *g_009bbff0 = 1;
    int code;
    if (e1 == 0) {
        code = 0xcb;
    } else {
        int idx3 = a2 * 0x167;
        int e3 = g_00946a50[idx3];
        code = 0xcc;
        if (e3 == 0) {
            code = 0xcb;
        }
    }
    int *table = reinterpret_cast<int *>(*g_009b90f8);
    int str = table[code];
    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    return reinterpret_cast<char *>(strings->get(str));
}
