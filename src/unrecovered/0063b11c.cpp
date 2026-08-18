// ORIGINAL: 0x0063B11C sub_63b11c 0x0063B11C-0x0063B1BF FILE
// working copy - scaffold materialised by --work
// size      163 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63b11c  at 0x0063B11C  (163 bytes)
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
static int *const g_00698b78 = (int *)0x00698B78;
// SIGNATURE CHANGED: `ret` with no operand and 11 distinct [esp+N] reads
// before any push - the zero-arg fallback can't reach any of them.
extern "C" void __cdecl sub_63b11c(int a1, int a2, int a3, int a4, int a5, int a6,
                                     int a7, int a8, int a9, int a10, int a11) {
    if (a5 > 0 && a6 > 0) {
        *g_00698b78 = a10 << 1;
        *g_00698b70 = a1 + (a4 * a10 + a3) * 2;
        unsigned int accY = 0;
        do {
            unsigned int accX = 0;
            int rowCount = a5;
            unsigned short *dst = reinterpret_cast<unsigned short *>(*g_00698b70);
            do {
                if (*reinterpret_cast<char *>(a2 + (accX >> 16)) != 0) {
                    *dst = *reinterpret_cast<unsigned short *>(a11 + (*dst) * 2);
                }
                dst++;
                accX += a7;
                rowCount--;
            } while (rowCount != 0);
            *g_00698b70 = *g_00698b70 + *g_00698b78;
            a2 = a2 + ((accY + a8) >> 16) * a9;
            accY = (accY + a8) & 0xffff;
            a6--;
        } while (a6 != 0);
    }
}
