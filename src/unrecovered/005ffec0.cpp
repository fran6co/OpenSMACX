// ORIGINAL: 0x005FFEC0 sub_5ffec0 0x005FFEC0-0x005FFF8B FILE
// working copy - scaffold materialised by --work
// size      203 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x005FFF0B 0x005FFF24 0x005FFF51 0x005FFF80

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ffec0  at 0x005FFEC0  (203 bytes)
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
static int *const g_006692ac = (int *)0x006692AC;
static int *const g_006692b0 = (int *)0x006692B0;
static int *const g_009b8190 = (int *)0x009B8190;
static int *const g_009b8194 = (int *)0x009B8194;
typedef int (__stdcall *DefWindowProcFn)(int, int, int, int);
typedef int (__stdcall *CallWindowProcFn)(int, int, int, int, int);

extern "C" int __stdcall sub_5ffec0(int a1, int a2, int a3, int a4) {
    switch (a2) {
    case 0x100:
        (*reinterpret_cast<DefWindowProcFn *>(g_006692b0))(*g_009b8194, 0x100, a3, a4);
        return 0;
    case 0x201:
        (*reinterpret_cast<DefWindowProcFn *>(g_006692b0))(*g_009b8194, 0x201, a3, a4);
        return 0;
    case 0x3b9: {
        int *p = reinterpret_cast<int *>(*g_009b8190);
        if (p != 0) {
            *p = *p & 0xfffffffe;
        }
        break;
    }
    default:
        break;
    }
    int *cur = reinterpret_cast<int *>(*g_009b8190);
    if (cur != 0) {
        int proc = *reinterpret_cast<int *>(reinterpret_cast<char *>(cur) + 0x464);
        return (*reinterpret_cast<CallWindowProcFn *>(g_006692ac))(proc, a1, a2, a3, a4);
    }
    return (*reinterpret_cast<DefWindowProcFn *>(g_006692b0))(a1, a2, a3, a4);
}
