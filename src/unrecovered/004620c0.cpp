// ORIGINAL: 0x004620C0 FILE
// RULED-OUT: nullary contract head (reads [ebp+8]/[ebp+0xc], no ecx
//            use -> 2 plain int args, not __thiscall). First
//            divergence #2 push/mov, 176 vs 196 B.
// working copy - scaffold materialised by --work
// name      sub_4620c0
// size      196 bytes
// spans     0x004620C0-0x00462184
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4620c0  at 0x004620C0  (196 bytes)
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
static int *const g_00949870 = (int *)0x00949870;
static int *const g_00949874 = (int *)0x00949874;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
extern "C" int __cdecl sub_4620c0(int a1, int a2) {
    if (a2 < 0 || a2 >= *g_00949874) {
        return 0;
    }
    if (a1 < 0 || a1 >= *g_00949870) {
        return 0;
    }
    int wrapFlag = *g_0094988c & 1;
    int width = *g_00949870;
    unsigned char *terrainBase = reinterpret_cast<unsigned char *>(*g_0094a30c);
    for (int off = 0; off < 0x20; off += 4) {
        int dx = *reinterpret_cast<int *>(0x0066EF50 + off);
        int x = dx + a1;
        if (!wrapFlag) {
            if (x < 0) {
                x = width + x;
            } else if (x >= width) {
                x -= width;
            }
        }
        int dy = *reinterpret_cast<int *>(0x0066EF74 + off);
        int y = dy + a2;
        if (y >= 0 && y < *g_00949874 && x >= 0 && x < width) {
            int idx = *g_0068faf0 * y + (x >> 1);
            unsigned char cell = terrainBase[idx * 0x2c];
            if ((cell & 0xe0) >= 0x60) {
                return 1;
            }
        }
    }
    return 0;
}
