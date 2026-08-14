// ORIGINAL: 0x00643EB0 BYTE_EXACT FILE
// LEVER: `if (rc != 0) { ...; return; } continue;` early-return guard for
//        the FIRST huft_build check, not `if (rc == 0) {success} else
//        {error}` - the if/else form placed the error block at the end of
//        the function (wrong fall-through), the early return puts it right
//        after the call, matching. The nullary-declared huft_build/sub_644100
//        callees needed differently-named `extern "C"` redeclarations with
//        the real (8-arg / 2-arg) signatures.
// working copy - scaffold materialised by --work
// name      sub_643eb0
// size      232 bytes
// spans     0x00643EB0-0x00643F98
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006439F0 0x00644100

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_643eb0  at 0x00643EB0  (232 bytes)
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
extern "C" int __cdecl huft_build();
extern "C" int __cdecl sub_644100();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006a5a90 = (int *)0x006A5A90;
static int *const g_006a5b0c = (int *)0x006A5B0C;
static int *const g_006a5b88 = (int *)0x006A5B88;
static int *const g_006a5c00 = (int *)0x006A5C00;
static int *const g_006a5cc4 = (int *)0x006A5CC4;
static int *const g_006a5ce4 = (int *)0x006A5CE4;
extern "C" int __cdecl huft_build_impl(int, int, int, void *, void *, int *, int, int);
extern "C" void __cdecl sub_644100_impl(int, int);

extern "C" int __cdecl sub_643eb0(int p1, int p2, int p3, int p4, int p5, int *p6, int *p7, int p8) {
    int rc = huft_build_impl(p3, p1, 0x101, g_006a5a90, g_006a5b0c, p6, p4, p8);
    if (rc != 0) {
        if (rc == -3) {
            *reinterpret_cast<int **>(p8 + 0x18) = g_006a5ce4;
            return -3;
        }
        if (rc == -5) {
            sub_644100_impl(*p6, p8);
            *reinterpret_cast<int **>(p8 + 0x18) = g_006a5cc4;
            rc = -3;
        }
        return rc;
    }
    rc = huft_build_impl(p3 + p1 * 4, p2, 0, g_006a5b88, g_006a5c00, p7, p5, p8);
    if (rc != 0) {
        if (rc == -3) {
            *reinterpret_cast<int **>(p8 + 0x18) = g_006a5ce4;
        } else if (rc == -5) {
            sub_644100_impl(*p7, p8);
            *reinterpret_cast<int **>(p8 + 0x18) = g_006a5cc4;
            rc = -3;
        }
        sub_644100_impl(*p6, p8);
        return rc;
    }
    return 0;
}
