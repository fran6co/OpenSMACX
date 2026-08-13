// ORIGINAL: 0x005ABFF0 BYTE_EXACT FILE
// working copy - scaffold materialised by --work
// name      ?get_rating@@YAHHH@Z
// size      97 bytes
// spans     0x005ABFF0-0x005AC051
// prototype int (__cdecl ?get_rating@@YAHHH@Z)(int factionID, int)
// callers   3   call targets   0
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_rating@@YAHHH@Z  at 0x005ABFF0  (97 bytes)
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
static int *const g_009a649c = (int *)0x009A649C;

// Indexed table base: the pointer would let /O2 fold the address into the
// index arithmetic (lea -> add), which is not what the original does.
extern int g_faction_table[];

int __cdecl get_rating(int a1, int a2) {
    int value = g_faction_table[a1 * 2099];
    int rating = value;
    for (int i = 3; i <= value; i++) {
        rating += (i - 1) / 2;
    }
    if (*(unsigned char *)g_009a649c & 0x40) {
        rating++;
    }
    return (rating + 6) * a2 / 0xa0;
}
