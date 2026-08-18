// ORIGINAL: 0x005B61A0 sub_5b61a0 0x005B61A0-0x005B6259 FILE
// working copy - scaffold materialised by --work
// size      185 bytes
// prototype int (__cdecl sub_5B61A0)(int vehID)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5b61a0  at 0x005B61A0  (185 bytes)
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
int has_abil(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0094a30c = (int *)0x0094A30C;
extern char g_952828_recs[];

extern "C" int __cdecl sub_5b61a0(int a1) {
    char *rec = g_952828_recs + a1 * 0x34;
    short f0A = *reinterpret_cast<short *>(rec + 0xA);
    if (has_abil(f0A, 0x40) != 0) {
        return 1;
    }
    f0A = *reinterpret_cast<short *>(rec + 0xA);
    if (has_abil(f0A, 4) != 0) {
        return 1;
    }
    unsigned short f08 = *reinterpret_cast<unsigned short *>(rec + 0x8);
    if ((f08 & 0x400) != 0) {
        return 1;
    }
    short f02 = *reinterpret_cast<short *>(rec + 0x2);
    short f00 = *reinterpret_cast<short *>(rec + 0x0);
    int idx = f02 * (*g_0068faf0) + (f00 >> 1);
    char *mapbase = *reinterpret_cast<char **>(g_0094a30c);
    char *tile = mapbase + idx * 0x2C;
    unsigned char flags = *reinterpret_cast<unsigned char *>(tile + 8);
    if ((flags & 0x20) != 0) {
        unsigned char lowbyte = *reinterpret_cast<unsigned char *>(tile);
        if ((lowbyte & 0xE0) >= 0x40 && *reinterpret_cast<short *>(rec + 0xA) != 0x13) {
            return 2;
        }
    }
    return 0;
}
