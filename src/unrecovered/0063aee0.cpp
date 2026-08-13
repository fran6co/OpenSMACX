// ORIGINAL: 0x0063AEE0 FILE
// RULED-OUT: `int` return (no eax set before final ret, changed to `void`);
//            address range carries a _SELFMOD (copy-protection) span, so the
//            harness's authoritative check REFUSES any body here regardless
//            of content - MISMATCH is the best obtainable signal via --dir.
// working copy - scaffold materialised by --work
// name      sub_63aee0
// size      109 bytes
// spans     0x0063AEE0-0x0063AEF4;0x0066731C-0x00667375
// prototype 
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0066638F 0x00666989 0x00666F68

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63aee0  at 0x0063AEE0  (109 bytes)
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
extern "C" int __cdecl sub_66638f();
extern "C" int __cdecl sub_666989();
extern "C" int __cdecl sub_666f68();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066710e = (int *)0x0066710E;
static int *const g_0066711f = (int *)0x0066711F;
static int *const g_00667125 = (int *)0x00667125;
static int *const g_0066712b = (int *)0x0066712B;
static int *const g_00667131 = (int *)0x00667131;
static int *const g_0066724b = (int *)0x0066724B;
static int *const g_0066725c = (int *)0x0066725C;
static int *const g_006672b0 = (int *)0x006672B0;
static int *const g_006672b8 = (int *)0x006672B8;
static int *const g_006672be = (int *)0x006672BE;
static int *const g_006672c4 = (int *)0x006672C4;
static int *const g_006672ca = (int *)0x006672CA;
static int *const g_00667305 = (int *)0x00667305;
static int *const g_0066730d = (int *)0x0066730D;
static int *const g_009be6ac = (int *)0x009BE6AC;
static int *const g_009be6b8 = (int *)0x009BE6B8;
static int *const g_009c082c = (int *)0x009C082C;
extern "C" void __cdecl sub_63aee0() {
    sub_666989();
    sub_66638f();
    sub_666f68();

    int v1 = *g_009c082c;
    *g_0066724b = v1;
    *g_0066710e = v1;
    *g_00667305 = v1;
    *g_006672b0 = v1;

    int v2 = *g_009be6ac;
    *g_0066725c = v2;
    *g_0066711f = v2;
    *g_0066730d = v2;
    *g_006672b8 = v2;

    v2++;
    *g_00667125 = v2;
    *g_006672be = v2;

    v2 += *g_009be6b8;
    *g_00667131 = v2;
    *g_006672ca = v2;

    v2--;
    *g_0066712b = v2;
    *g_006672c4 = v2;
}
