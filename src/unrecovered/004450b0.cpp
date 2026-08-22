// ORIGINAL: 0x004450B0 sub_4450b0 0x004450B0-0x0044511C FILE
// TRIED: plain constructor with raw pointer-offset writes (eax=ecx=this, returns this implicitly). Diverges at instr #2: original pushes ecx for a compiler-generated [ebp-4] unwind-context local (a partially-constructed-object marker) that a plain non-throwing body never allocates.
// working copy - scaffold materialised by --work
// size      108 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4450b0  at 0x004450B0  (108 bytes)
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
static int *const g_006693a0 = (int *)0x006693A0;
static int *const g_006693a4 = (int *)0x006693A4;
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006698c0 = (int *)0x006698C0;
static int *const g_006698c4 = (int *)0x006698C4;
static int *const g_0066b0ec = (int *)0x0066B0EC;
static int *const g_009b3374 = (int *)0x009B3374;
class Obj4450b0 { public:
    Obj4450b0(int a1);
};

Obj4450b0::Obj4450b0(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 != 0) {
        *reinterpret_cast<int **>(self + 4) = g_0066b0ec;
        *reinterpret_cast<int **>(self + 0x28) = g_006693ac;
        int head = *g_009b3374;
        *reinterpret_cast<int *>(self + 0x2c) = head;
        *g_009b3374 = 0;
    }
    int f4 = *reinterpret_cast<int *>(self + 4);
    *reinterpret_cast<int **>(self) = g_006693a4;
    int inner = *reinterpret_cast<int *>(f4 + 4);
    *reinterpret_cast<int *>(inner + reinterpret_cast<int>(self) + 4) = reinterpret_cast<int>(g_006693a0);
    *reinterpret_cast<int *>(self + 8) = 0;
    *reinterpret_cast<int *>(self + 0xc) = 0;
    *reinterpret_cast<int *>(self + 0x10) = 0;
    *reinterpret_cast<int *>(self + 0x14) = 0;
    *reinterpret_cast<int *>(self + 0x18) = 0;
    int f4b = *reinterpret_cast<int *>(self + 4);
    *reinterpret_cast<int **>(self) = g_006698c4;
    int inner2 = *reinterpret_cast<int *>(f4b + 4);
    *reinterpret_cast<int *>(inner2 + reinterpret_cast<int>(self) + 4) = reinterpret_cast<int>(g_006698c0);
}
