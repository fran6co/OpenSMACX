// ORIGINAL: 0x0057DF30 sub_57df30 0x0057DF30-0x0057DFB2 FILE
// RULED-OUT: a separate pointer variable for the RMW counter slot ('mov' then a stray 'lea' to materialise the address) - index into the extern array directly (`table[idx]`) instead, which drops the divergence from #10 to #22. MISMATCH remains open.
// working copy - scaffold materialised by --work
// size      130 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_57df30  at 0x0057DF30  (130 bytes)
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
static int *const g_0096ea6c = (int *)0x0096EA6C;
extern "C" unsigned char g_table_9ab888[];
extern "C" unsigned char g_table_94b558[];

extern "C" void __cdecl sub_57df30(int a1, int a2) {
    int index = a1 * 2099;
    int32_t val = g_0096ea6c[index];
    int32_t v = (val + 1) % 64;
    g_0096ea6c[index] = v;

    unsigned char *src = g_table_9ab888 + a2 * 0x34;
    unsigned char *dst = g_table_94b558 + (a1 * 0x40 + val) * 8;

    *(int32_t *)dst = *(int32_t *)src;
    dst[4] = src[4];
    dst[5] = src[5];
    dst[6] = src[6];
    dst[7] = src[7];
}
