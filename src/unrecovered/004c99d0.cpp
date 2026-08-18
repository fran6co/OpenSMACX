// ORIGINAL: 0x004C99D0 sub_4c99d0 0x004C99D0-0x004C9A43 FILE
// working copy - scaffold materialised by --work
// size      115 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c99d0  at 0x004C99D0  (115 bytes)
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
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0094a30c = (int *)0x0094A30C;
struct TileEntry {
    unsigned char field0_;
    unsigned char pad1_[4];
    unsigned char field5_;
    unsigned char pad6_[2];
    unsigned int flags_;
    unsigned char pad_rest_[0x2c - 12];
};

extern int g_terrain_table[8][8];

extern "C" int __cdecl sub_4c99d0(int a1, int a2) {
    int index = *g_0068faf0 * a2 + (a1 >> 1);
    TileEntry *base = *reinterpret_cast<TileEntry **>(g_0094a30c);
    TileEntry *tile = base + index;
    unsigned int flags = tile->flags_;
    int result = g_terrain_table[((flags & 4) != 0) + 5][0];
    if ((flags & 0x80) != 0) {
        result++;
    }
    if ((flags & 0x20) != 0 && (tile->field0_ & 0xE0) >= 0x40) {
        result += 2;
    }
    if ((flags & 0x200000) != 0) {
        return result + 2;
    }
    return result + (tile->field5_ >> 6);
}
