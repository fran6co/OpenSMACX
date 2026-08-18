// ORIGINAL: 0x0057DFC0 ?is_bunged@@YA_NHHHHHH@Z 0x0057DFC0-0x0057E037 FILE
// working copy - scaffold materialised by --work
// size      119 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?is_bunged@@YA_NHHHHHH@Z  at 0x0057DFC0  (119 bytes)
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
static int *const g_0094b55d = (int *)0x0094B55D;
static int *const g_009a64e8 = (int *)0x009A64E8;
bool __cdecl is_bunged(int a1, int a2, int a3, int a4, int a5, int a6) {
    unsigned char flag_byte = *reinterpret_cast<unsigned char *>(g_009a64e8);
    unsigned char shifted = static_cast<unsigned char>(1 << (a1 & 0x1f));
    if ((shifted & flag_byte) != 0) {
        unsigned char *p = reinterpret_cast<unsigned char *>(g_0094b55d) + a1 * 0x200;
        int i;
        for (i = 0; i < 0x40; ++i) {
            if (p[-1] == a2 && p[0] == a3 && p[1] == a4 &&
                *reinterpret_cast<int *>(p - 5) == a5 && p[2] == a6) {
                return true;
            }
            p += 8;
        }
    }
    return false;
}
