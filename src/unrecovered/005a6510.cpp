// ORIGINAL: 0x005A6510 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      sub_5a6510
// size      132 bytes
// spans     0x005A6510-0x005A6594
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5a6510  at 0x005A6510  (132 bytes)
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
// `extern T name[]` rather than a fixed `T *const` - the address itself does
// no work here (plain element loads), so either spelling matches, and the
// extern form reads better.
extern int g_0066ef50[9];
extern int g_0066ef74[9];

extern "C" int __cdecl sub_5a6510(int a1, int a2) {
    int sign1;
    if (a1 > 0) {
        sign1 = 1;
    } else {
        sign1 = (a1 >= 0) - 1;
    }
    int sign2;
    if (a2 > 0) {
        sign2 = 1;
    } else {
        sign2 = (a2 >= 0) - 1;
    }
    for (int i = 0; i < 9; i++) {
        int x = g_0066ef50[i];
        int signx;
        if (x > 0) {
            signx = 1;
        } else {
            signx = (x >= 0) - 1;
        }
        if (sign1 == signx) {
            int y = g_0066ef74[i];
            int signy;
            if (y > 0) {
                signy = 1;
            } else {
                signy = (y >= 0) - 1;
            }
            if (sign2 == signy) {
                return i;
            }
        }
    }
    return -1;
}
