// ORIGINAL: 0x004F0650 ?upgrade_any_prototypes@@YAXH@Z 0x004F0650-0x004F06D3 FILE
// RULED-OUT: manual mask/sign-fixup for idx>=0x40 case (jns vs jge) - plain `idx % 0x40` reproduced the and/jns/dec/or/inc block exactly; last divergence is `(flags & 0x100)` compiling to a bare test instead of original's and+test cx,cx
// working copy - scaffold materialised by --work
// size      131 bytes
// prototype void (__cdecl ?upgrade_any_prototypes@@YAXH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004F0460

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?upgrade_any_prototypes@@YAXH@Z  at 0x004F0650  (131 bytes)
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
void upgrade_prototypes(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090ea3c = (int *)0x0090EA3C;
static int *const g_009a6494 = (int *)0x009A6494;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab898 = (int *)0x009AB898;
void __cdecl upgrade_any_prototypes(int a1) {
    int i;
    *g_0090ea3c = 0;
    if (((unsigned char)(1 << a1) & *reinterpret_cast<unsigned char *>(g_009a64e8)) == 0 ||
        (*g_009a6494 & 0x100000) != 0) {
        i = 0;
        do {
            int idx = i;
            if (idx >= 0x40) {
                idx = idx % 0x40 + a1 * 0x40;
            }
            unsigned short flags = reinterpret_cast<unsigned short *>(g_009ab898)[idx * 0x1a];
            if ((flags & 1) != 0 && (flags & 4) != 0 && (flags & 0x100) != 0) {
                upgrade_prototypes(a1, idx);
            }
            i = i + 1;
        } while (i < 0x80);
    }
}
