// ORIGINAL: 0x0044B070 ?hof_load@@YAHXZ 0x0044B070-0x0044B0F8 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      136 bytes
// prototype 
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645598 0x00645646 0x00646178

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hof_load@@YAHXZ  at 0x0044B070  (136 bytes)
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

struct FILE;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl _fread(void*, size_t, size_t, FILE*);
extern "C" int __cdecl fclose();
extern "C" int __cdecl fopen();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00684620 = (int *)0x00684620;
static int *const g_00684624 = (int *)0x00684624;
static int *const g_00759194 = (int *)0x00759194;
static int *const g_009a3840 = (int *)0x009A3840;
static int *const g_009a4b64 = (int *)0x009A4B64;
extern "C" FILE *__cdecl hof_fopen(const char *, const char *);
extern "C" int __cdecl hof_fclose(FILE *);
extern unsigned char g_009a3840_arr[];

int __cdecl hof_load() {
    FILE *f = hof_fopen(reinterpret_cast<const char *>(g_00684624),
                         reinterpret_cast<const char *>(g_00684620));
    if (f == 0) {
        unsigned char *p = g_009a3840_arr;
        do {
            int n = 5;
            do {
                *p = 0;
                p += 0x8c;
                n--;
            } while (n != 0);
        } while (reinterpret_cast<int>(p) < reinterpret_cast<int>(g_009a4b64));
        return 1;
    }
    *g_00759194 = 0;
    int n = _fread(g_009a3840, 0x1324, 1, f);
    if (n == 0) {
        unsigned char *p = g_009a3840_arr;
        do {
            int n2 = 5;
            do {
                *p = 0;
                p += 0x8c;
                n2--;
            } while (n2 != 0);
        } while (reinterpret_cast<int>(p) < reinterpret_cast<int>(g_009a4b64));
    }
    hof_fclose(f);
    return 0;
}
