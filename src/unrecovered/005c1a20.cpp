// ORIGINAL: 0x005C1A20 FILE
// RULED-OUT: nesting the a1<0 guard as `if (a1>=0){...} return 0;` instead
//            of an early return - same 'jl' polarity either way (/O2
//            canonicalises both to the same CFG); the rest of the OR-chain
//            and both later guards already match
// working copy - scaffold materialised by --work
// name      ?veh_selectable@@YAHH@Z
// size      138 bytes
// spans     0x005C1A20-0x005C1AAA
// prototype int (__cdecl ?veh_selectable@@YAHH@Z)(int vehID)
// callers   11   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1C40

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?veh_selectable@@YAHH@Z  at 0x005C1A20  (138 bytes)
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
int veh_jail(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
int __cdecl veh_selectable(int a1) {
    if (a1 < 0) {
        return 0;
    }
    int off = a1 * 0x34;
    int f2a = *reinterpret_cast<short *>(0x0095282a + off);
    int f28 = *reinterpret_cast<short *>(0x00952828 + off);
    if (f2a < 0 || f2a >= *g_00949874 || f28 < 0 || f28 >= *g_00949870) {
        return 0;
    }
    if (*reinterpret_cast<unsigned char *>(0x00952836 + off) != *g_00939284) {
        return 0;
    }
    unsigned short flags = *reinterpret_cast<unsigned short *>(0x00952830 + off);
    if ((flags & 0x40) != 0 && (flags & 0x400) != 0) {
        return 0;
    }
    return veh_jail(a1) == 0;
}
