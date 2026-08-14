// ORIGINAL: 0x005CD2D0 FILE
// RULED-OUT: `long long` (VC6 has none, use __int64); flat 64-iter loop
//            instead of nested 8x8 do/while counted down
// working copy - scaffold materialised by --work
// name      sub_5cd2d0
// size      210 bytes
// spans     0x005CD2D0-0x005CD3A2
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006458C8

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cd2d0  at 0x005CD2D0  (210 bytes)
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
extern "C" int __cdecl _ftol();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066d8d0 = (int *)0x0066D8D0;
static int *const g_0066fc1c = (int *)0x0066FC1C;
static int *const g_0066fc20 = (int *)0x0066FC20;
static int *const g_00691f4c = (int *)0x00691F4C;
extern "C" int __cdecl sub_5cd2d0(int *a1, int a2) {
    static int *const g_0069204c = (int *)0x0069204C;

    if (a2 == 100) {
        int *p = a1;
        int outer = 8;
        do {
            int inner = 8;
            do {
                int idx = static_cast<int>(p - a1);
                __int64 v = static_cast<__int64>(g_00691f4c[idx]) * 0x80000;
                *p = static_cast<int>(v >> 16);
                p++;
                inner--;
            } while (inner != 0);
            outer--;
        } while (outer != 0);
    } else {
        float scale = static_cast<float>((*reinterpret_cast<double *>(g_0066fc20) - static_cast<double>(a2)) * (*reinterpret_cast<float *>(g_0066d8d0)));
        char *dst = reinterpret_cast<char *>(a1);
        int off = 0;
        do {
            int inner = 8;
            do {
                double t = static_cast<double>(*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0069204c) + off));
                int tableVal = *reinterpret_cast<int *>(reinterpret_cast<char *>(g_00691f4c) + off);
                int yi = static_cast<int>(t * scale * (*reinterpret_cast<float *>(g_0066fc1c)));
                __int64 v = static_cast<__int64>(tableVal) * static_cast<__int64>(yi);
                *reinterpret_cast<int *>(dst + off) = static_cast<int>(v >> 16);
                off += 4;
                inner--;
            } while (inner != 0);
        } while (off < 0x100);
    }

    __int64 v = static_cast<__int64>(*g_00691f4c) * 0x80000;
    *a1 = static_cast<int>(v >> 16);
    return 0;
}
