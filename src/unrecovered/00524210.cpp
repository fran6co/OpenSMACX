// ORIGINAL: 0x00524210 FILE
// working copy - scaffold materialised by --work
// name      ?climactic_changes@@YAXXZ
// size      176 bytes
// spans     0x00524210-0x005242C0
// prototype 
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0048C0A0 0x005C5A30

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?climactic_changes@@YAXXZ  at 0x00524210  (176 bytes)
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
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void draw_map(int);
void world_climate();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068b594 = (int *)0x0068B594;
static int *const g_0068b5a4 = (int *)0x0068B5A4;
static int *const g_0068b5b0 = (int *)0x0068B5B0;
static int *const g_0068b5c0 = (int *)0x0068B5C0;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_0094987c = (int *)0x0094987C;
static int *const g_009a67cc = (int *)0x009A67CC;
static int *const g_009a67d0 = (int *)0x009A67D0;
static int *const g_009a67d4 = (int *)0x009A67D4;
static int *const g_009a67d8 = (int *)0x009A67D8;
void __cdecl climactic_changes() {
    int sign;

    if (*g_009a67d8 != 0) {
        *g_009a67d4 += *g_009a67cc;
        if (*g_009a67d0 <= *g_009a67d4) {
            *g_009a67d4 -= *g_009a67d0;
            if (*g_009a67d8 > 0) {
                sign = 1;
            } else {
                sign = (*g_009a67d8 >= 0) - 1;
            }
            *g_009a67d8 -= sign;
            *g_0094987c += sign;
            world_climate();
            draw_map(1);
            if (sign > 0) {
                popp((char *)*g_00691b0c, (const char *)g_0068b5a4, 0,
                     (const char *)g_0068b594, 0);
            } else {
                popp((char *)*g_00691b0c, (const char *)g_0068b5c0, 0,
                     (const char *)g_0068b5b0, 0);
            }
        }
    }
}
