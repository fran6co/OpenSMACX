// ORIGINAL: 0x0058FBC0 ?get_his_her@@YAPADHH@Z 0x0058FBC0-0x0058FC4B FILE
// working copy - scaffold materialised by --work
// size      139 bytes
// prototype 
// callers   4   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645460
// indirect  0x0058FC23 0x0058FC3E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_his_her@@YAPADHH@Z  at 0x0058FBC0  (139 bytes)
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

extern "C" char *strcpy(char *, const char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b90d8 = (int *)0x009B90D8;

// `strcpy` is compiler-recognised and gets expanded inline (a manual
// strlen-style scan) unless told not to - the original calls the real
// `_strcpy`, so force that here too.
#pragma function(strcpy)

typedef int (__stdcall *CharUpperAFn)(char *);
static CharUpperAFn *const g_pCharUpperA = (CharUpperAFn *)0x0066931C;

extern int g_946a50_tbl[];
extern int *g_9b90f8_arr;
extern char g_945838_buf[];

char * __cdecl get_his_her(int a1, int a2) {
    int idx = a1 * 359;
    int val = g_9b90f8_arr[(g_946a50_tbl[idx] != 0) + 0xcf];
    char *str = (char *)reinterpret_cast<Strings *>(g_009b90d8)->get(val);
    strcpy(g_945838_buf, str);
    if (a2 != 0) {
        if (a2 == 1) {
            char buf[2];
            buf[0] = g_945838_buf[0];
            buf[1] = 0;
            (*g_pCharUpperA)(buf);
            g_945838_buf[0] = buf[0];
            return g_945838_buf;
        }
        (*g_pCharUpperA)(g_945838_buf);
    }
    return g_945838_buf;
}
