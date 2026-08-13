// ORIGINAL: 0x004D5FF0 FILE
// working copy - scaffold materialised by --work
// name      sub_4d5ff0
// size      126 bytes
// spans     0x004D5FF0-0x004D606E
// prototype 
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00628A50 0x0064FD20

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4d5ff0  at 0x004D5FF0  (126 bytes)
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
extern "C" int __cdecl _strcmpi();
void swap(int *, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_009a64cc = (int *)0x009A64CC;
extern "C" int __cdecl strcmpi_impl(const char *, const char *);

extern "C" void __cdecl sub_4d5ff0(int *param_1) {
    bool did_swap;
    do {
        did_swap = false;
        int *p = param_1;
        int n = *g_009a64cc;
        if (n == 1 || n - 1 < 0) {
            return;
        }
        int i = 0;
        do {
            const char *row0 = reinterpret_cast<const char *>(g_0097d053) + p[0] * 0x134;
            const char *row1 = reinterpret_cast<const char *>(g_0097d053) + p[1] * 0x134;
            if (strcmpi_impl(row0, row1) > 0) {
                did_swap = true;
                swap(p, p + 1);
            }
            i = i + 1;
            p = p + 1;
        } while (i < n - 1);
    } while (did_swap);
}
