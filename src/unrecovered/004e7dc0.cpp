// ORIGINAL: 0x004E7DC0 ?resource_yield@@YAHHHHHH@Z 0x004E7DC0-0x004E7E34 FILE
// RULED-OUT: `case 3: ...; return 0;` (same size, 16 bytes over); the extra bytes are in the switch/bounds-check shape, not the case body.
// working copy - scaffold materialised by --work
// size      116 bytes
// prototype int (__cdecl ?resource_yield@@YAHHHHHH@Z)(int type, int factionID, int baseID, int xCoord, int yCoord)
// callers   7   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?resource_yield@@YAHHHHHH@Z  at 0x004E7DC0  (116 bytes)
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
int crop_yield(int, int, int, int, int);
int energy_yield(int, int, int, int, int);
int mine_yield(int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_004e7e34 = (int *)0x004E7E34;
static int *const g_0090e998 = (int *)0x0090E998;
int __cdecl resource_yield(int a1, int a2, int a3, int a4, int a5) {
    switch (a1) {
        case 0:
            return crop_yield(a2, a3, a4, a5, 0);
        case 1:
            return mine_yield(a2, a3, a4, a5, 0);
        case 2:
            return energy_yield(a2, a3, a4, a5, 0);
        case 3:
            *g_0090e998 = 0;
            break;
    }
    return 0;
}
