// ORIGINAL: 0x004CE6B0 FILE
// RULED-OUT: caching the recomputed table index in one local (idx) instead
//            of re-reading g_0068faf0 fresh each use loses the frame; kept
//            `half = a1>>1` cached (matches the original's stack-slot
//            reuse of the a2 parameter) but still diverges on the imul
//            operand ordering a few instructions in.
// working copy - scaffold materialised by --work
// name      sub_4ce6b0
// size      221 bytes
// spans     0x004CE6B0-0x004CE78D
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF40 0x00591D60 0x00591E50 0x005C2020

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4ce6b0  at 0x004CE6B0  (221 bytes)
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
void bit_set(int, int, int, int);
void draw_tile(int, int, int);
void synch_bit(int, int, int);
void world_alt_set(int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_006919ec = (int *)0x006919EC;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_009a64c0 = (int *)0x009A64C0;

extern unsigned int tbl_96c9e0[];

extern "C" void __cdecl sub_4ce6b0(int a1, int a2) {
    bit_set(a1, a2, 0x20, 1);
    bit_set(a1, a2, 0x28805c, 0);
    bit_set(a1, a2, *g_006919ec, 0);

    unsigned char *base = *reinterpret_cast<unsigned char **>(g_0094a30c);
    int half = a1 >> 1;
    unsigned char flags = base[(*g_0068faf0 * a2 + half) * 0x2c];
    if ((flags & 0xe0) < 0x40) {
        world_alt_set(a1, a2, 2, 0);
    }

    int field = *g_00939284;
    unsigned int entry = tbl_96c9e0[field * 0x833];
    unsigned char flags2 = base[(*g_0068faf0 * a2 + half) * 0x2c + 4];
    unsigned char g = *reinterpret_cast<unsigned char *>(g_009a64c0);
    if ((entry & 0x200) != 0
        || (flags2 & (unsigned char)(1 << (field & 0x1f))) != 0
        || (g & 0x80) != 0) {
        synch_bit(a1, a2, field);
        draw_tile(a1, a2, 2);
    }
}
