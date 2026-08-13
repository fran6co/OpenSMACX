// ORIGINAL: 0x00639630 FILE
// working copy - scaffold materialised by --work
// name      sub_639630
// size      100 bytes
// spans     0x00639630-0x00639694
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0063963D 0x0063964E 0x0063965F 0x00639670 0x0063967A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_639630  at 0x00639630  (100 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009be69c = (int *)0x009BE69C;
static int *const g_009c0b84 = (int *)0x009C0B84;
extern "C" void __cdecl sub_639630(int param_1) {
    typedef void (__cdecl *FreeFunc)(int);

    if (*(int *)(param_1 + 0x50) != 0) {
        ((FreeFunc)*g_009c0b84)(*(int *)(param_1 + 0x50));
    }
    if (*(int *)(param_1 + 0x54) != 0) {
        ((FreeFunc)*g_009c0b84)(*(int *)(param_1 + 0x54));
    }
    if (*(int *)(param_1 + 0x60) != 0) {
        ((FreeFunc)*g_009c0b84)(*(int *)(param_1 + 0x60));
    }
    if (*(int *)(param_1 + 0x64) != 0) {
        ((FreeFunc)*g_009c0b84)(*(int *)(param_1 + 0x64));
    }
    ((FreeFunc)*g_009c0b84)(param_1);

    if (*g_009be69c != 0) {
        *g_009be69c = *g_009be69c - 1;
    }
}
