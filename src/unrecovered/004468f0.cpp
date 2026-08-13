// ORIGINAL: 0x004468F0 FILE
// working copy - scaffold materialised by --work
// name      sub_4468f0
// size      107 bytes
// spans     0x004468F0-0x0044695B
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4468f0  at 0x004468F0  (107 bytes)
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
static int *const g_007d3c3c = (int *)0x007D3C3C;
static int *const g_00949870 = (int *)0x00949870;
extern "C" int __cdecl sub_4468f0(int a1) {
    int base = *reinterpret_cast<int *>(*g_007d3c3c + 0x1dd9c);
    int iVar1 = base / 2;
    int n = *g_00949870;
    int iVar3 = n / 4;
    int iVar2 = iVar3 + iVar1;
    int half = a1 / 2;
    if (n / 2 < iVar2) {
        iVar2 = iVar2 - n / 2;
    }
    if (iVar1 < iVar3) {
        if (half <= iVar1 || iVar2 <= half) {
            return -1;
        }
    } else if (half < iVar1 && iVar2 < half) {
        return -1;
    }
    return 1;
}
