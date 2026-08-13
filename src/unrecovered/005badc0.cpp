// ORIGINAL: 0x005BADC0 FILE
// working copy - scaffold materialised by --work
// name      ?tech_secret@@YAXHH@Z
// size      148 bytes
// spans     0x005BADC0-0x005BAE54
// prototype void (__cdecl ?tech_secret@@YAXHH@Z)(int techID, int factionID)
// callers   0   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF310 0x00625E50 0x00625EC0 0x00645470 0x0064FC88

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tech_secret@@YAXHH@Z  at 0x005BADC0  (148 bytes)
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
extern "C" int __cdecl _itoa();
int __cdecl X_pop(const char *, int (__cdecl *)());
int parse_say(int, int, int, int);
int parse_says(int, char *, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006916f0 = (int *)0x006916F0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946d54 = (int *)0x00946D54;
static int *const g_0094f35c = (int *)0x0094F35C;
static int *const g_009b86a0 = (int *)0x009B86A0;
extern "C" char *__cdecl secret_itoa(int, char *, int);

void __cdecl tech_secret(int a1, int a2) {
    char buf[80];
    int idx1 = a1 * 0xb;
    int val1 = g_0094f35c[idx1];
    parse_say(0, val1, -1, -1);

    int idx2 = a2 * 0x167;
    char *str2 = reinterpret_cast<char *>(&g_00946d54[idx2]);
    parse_says(1, str2, -1, -1);

    char *dst = reinterpret_cast<char *>(g_009b86a0);
    *dst = 0;
    strcat(dst, reinterpret_cast<char *>(g_006916f0));

    int flag = (a2 != *g_00939284) ? 1 : 0;
    secret_itoa(flag, buf, 10);
    strcat(dst, buf);

    X_pop(dst, 0);
}
