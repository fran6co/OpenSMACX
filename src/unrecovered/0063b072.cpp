// ORIGINAL: 0x0063B072 sub_63b072 0x0063B072-0x0063B11C FILE
// working copy - scaffold materialised by --work
// size      170 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63b072  at 0x0063B072  (170 bytes)
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
static int *const g_00698b70 = (int *)0x00698B70;
static int *const g_00698b74 = (int *)0x00698B74;
static int *const g_00698b78 = (int *)0x00698B78;
// SIGNATURE CHANGED: `ret` with no operand and 10 distinct [esp+N] reads
// before any push.
// TRIED: zero-arg fallback signature - the image's 10 [esp+N] reads ahead of
//        any push are only reachable through the full __cdecl parameter list
extern "C" void __cdecl sub_63b072(int a1, int a2, int a3, int a4, int a5, int a6,
                                     int a7, int a8, int a9, short a10) {
    if (a5 > 0 && a6 > 0) {
        *g_00698b74 = a8 << 1;
        *g_00698b78 = a9 << 1;
        *g_00698b70 = a1 + (a4 * a9 + a3) * 2;
        unsigned int accY = 0;
        do {
            unsigned int accX = 0;
            int rowCount = a5;
            unsigned short *dst = reinterpret_cast<unsigned short *>(*g_00698b70);
            do {
                short val = *reinterpret_cast<short *>(a2 + (accX >> 16) * 2);
                if (val != a10) {
                    *dst = static_cast<unsigned short>(val);
                }
                dst++;
                accX += a7;
                rowCount--;
            } while (rowCount != 0);
            *g_00698b70 = *g_00698b70 + *g_00698b78;
            a2 = a2 + ((accY + a7) >> 16) * (*g_00698b74);
            accY = (accY + a7) & 0xffff;
            a6--;
        } while (a6 != 0);
    }
}
