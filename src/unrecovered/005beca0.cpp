// ORIGINAL: 0x005BECA0 FILE
// RULED-OUT: `g_00691b0c`/`g_00691b20` used directly as the string pointer
//            (they are PTR_s_ globals - one more dereference needed: a
//            `char*` stored AT that address, not the address of the string
//            itself). Original keeps a `push ebp; mov ebp, esp` frame this
//            source form never reproduces (register-pressure shape, not yet
//            found).
// working copy - scaffold materialised by --work
// name      ?X_text_open@@YAHPADPBD@Z
// size      119 bytes
// spans     0x005BECA0-0x005BED17
// prototype int (__cdecl ?X_text_open@@YAHPADPBD@Z)(int8* srcFileID, int8* sectionID)
// callers   19   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD550 0x00645660

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?X_text_open@@YAHPADPBD@Z  at 0x005BECA0  (119 bytes)
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
extern "C" int strcmp(const char *, const char *);
int text_open(char *, char *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00691b20 = (int *)0x00691B20;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a6488 = (int *)0x009A6488;
int __cdecl X_text_open(char * a1, const char * a2) {
    if (strcmp(a1, *reinterpret_cast<const char **>(g_00691b0c)) != 0) {
        return text_open(a1, (char *)a2);
    }
    char *table = reinterpret_cast<char *>(g_00946f58);
    int idx = *g_00939284;
    if ((table[idx * 0x59c] & 0x80) != 0 && *g_009a6488 != 0) {
        if (text_open(*reinterpret_cast<char **>(g_00691b20), (char *)a2) == 0) {
            return 0;
        }
    }
    return text_open(a1, (char *)a2);
}
