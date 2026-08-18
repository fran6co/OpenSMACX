// ORIGINAL: 0x005247B0 ?generators@@YAHHPAH@Z 0x005247B0-0x00524862 FILE
// working copy - scaffold materialised by --work
// size      178 bytes
// prototype int (__cdecl ?generators@@YAHHPAH@Z)(int factionID, int* count)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?generators@@YAHHPAH@Z  at 0x005247B0  (178 bytes)
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
void bitmask(int, int *, int *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_00949864 = (int *)0x00949864;
static int *const g_009a64cc = (int *)0x009A64CC;
extern unsigned char g_00946f58_arr[];
extern unsigned char g_0097d044_arr[];
extern unsigned char g_0097d046_arr[];
extern unsigned char g_0097d0cc_arr[];

int __cdecl generators(int a1, int * a2) {
    int count = 0;
    if (a2 != 0) {
        *a2 = 0;
    }
    if ((g_00946f58_arr[a1 * 0x59c] & 0x80) != 0) {
        int total = *g_009a64cc;
        if (total > 0) {
            int esi = 0;
            int i = 0;
            do {
                if ((int)g_0097d044_arr[esi] == a1) {
                    int idx = 0;
                    int mask = 0;
                    bitmask(0x28, &idx, &mask);
                    if ((g_0097d0cc_arr[esi + idx] & (unsigned char)mask) != 0) {
                        if ((signed char)g_0097d046_arr[esi] < *g_00949864) {
                            if (a2 != 0) {
                                (*a2)++;
                            }
                        } else {
                            count++;
                        }
                    }
                }
                i++;
                esi += 0x134;
            } while (i < total);
        }
        return count;
    }
    return 0;
}
