// ORIGINAL: 0x0063F9B0 sub_63f9b0 0x0063F9B0-0x0063FA72 FILE
// TRIED: nullary contract (evidence shows 1 stack param at [esp+0x10] after 3 pushes -> changed to sub_63f9b0(int param_1)); memcpy(27) for the tail rep-movsd/movsw/movsb copy. MISMATCH #1 push/mov, whole-function register order
// working copy - scaffold materialised by --work
// size      194 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0063F9C0 0x0063F9D1 0x0063F9E4 0x0063F9FA

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63f9b0  at 0x0063F9B0  (194 bytes)
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
static int *const g_00698df0 = (int *)0x00698DF0;
static int *const g_009c0b80 = (int *)0x009C0B80;
static int *const g_009c0b84 = (int *)0x009C0B84;
static int *const g_009c0d60 = (int *)0x009C0D60;
extern "C" void *memcpy(void *, const void *, unsigned int);

extern "C" int __cdecl sub_63f9b0(int param_1) {
    char *self = reinterpret_cast<char *>(param_1);
    typedef void (__cdecl *FreeFn)(int);
    typedef int (__cdecl *MallocFn)(int);

    if (*reinterpret_cast<int *>(self + 0x60) != 0) {
        (*reinterpret_cast<FreeFn *>(g_009c0b84))(*reinterpret_cast<int *>(self + 0x60));
    }
    if (*reinterpret_cast<int *>(self + 0x64) != 0) {
        (*reinterpret_cast<FreeFn *>(g_009c0b84))(*reinterpret_cast<int *>(self + 0x64));
    }
    int info = *reinterpret_cast<int *>(self + 4);
    int count = *reinterpret_cast<int *>(info + 0x10);
    int alloc1 = (*reinterpret_cast<MallocFn *>(g_009c0b80))(count << 2);
    *reinterpret_cast<int *>(self + 0x60) = alloc1;
    info = *reinterpret_cast<int *>(self + 4);
    count = *reinterpret_cast<int *>(info + 0x10);
    int alloc2 = (*reinterpret_cast<MallocFn *>(g_009c0b80))(count << 2);
    *reinterpret_cast<int *>(self + 0x64) = alloc2;

    if (*reinterpret_cast<int *>(self + 0x60) != 0 && alloc2 != 0) {
        info = *reinterpret_cast<int *>(self + 4);
        int base = *reinterpret_cast<int *>(info + 4);
        if (base == 0) {
            return 0;
        }
        int step = *reinterpret_cast<int *>(info + 0x18);
        int accum = 0;
        int i = 0;
        if (*reinterpret_cast<int *>(info + 0x10) > 0) {
            do {
                *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x60) + i * 4) = base;
                base += step * 2;
                *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x64) + i * 4) = accum;
                accum += *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 4) + 0x14);
                ++i;
            } while (i < *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 4) + 0x10));
        }
        return 0;
    }

    memcpy(g_009c0d60, g_00698df0, 27);
    return 1;
}
