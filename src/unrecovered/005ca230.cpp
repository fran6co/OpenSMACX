// ORIGINAL: 0x005CA230 FILE
// RULED-OUT: real signature is __fastcall(int *param_1, int param_2, int param_3)
//        (ecx/edx/stack), not the given __stdcall(int a1) - the given head
//        drops two of the three real parameters. This is mmio ring-buffer
//        bookkeeping: param_1[7]/[9] are byte offsets, param_1+0xdf a ring
//        counter, param_1+0xe0/0xe2 sibling counters bumped via
//        InterlockedIncrement.
// RULED-OUT: `<=`-with-sum-on-the-right vs `>=`-with-sum-on-the-left for the
//            wrap check (marginal effect only); could not reproduce the
//            entry "push ecx" dummy-stack-slot idiom VC6 uses ahead of the
//            callee-saved pushes.
// working copy - scaffold materialised by --work
// name      sub_5ca230
// size      216 bytes
// spans     0x005CA230-0x005CA308
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CA25F 0x005CA2ED 0x005CA2F6 0x005CA2F9

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ca230  at 0x005CA230  (216 bytes)
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
static int *const g_006690fc = (int *)0x006690FC;
static int *const g_0066936c = (int *)0x0066936C;
typedef long (__stdcall *MmioReadFn)(void *, char *, long);
typedef long (__stdcall *InterlockedIncrementFn)(long *);

extern "C" int __fastcall sub_5ca230(int *param_1, int param_2, int param_3) {
    if ((unsigned int)(param_2 + param_1[9]) >= (unsigned int)(param_1[7] + 1200000)) {
        param_1[9] = param_1[7];
    }

    void *handle = reinterpret_cast<void **>(param_1[0] + 0xa0)[0];
    long result = (*reinterpret_cast<MmioReadFn *>(g_0066936c))(handle, reinterpret_cast<char *>(param_1[9]), param_2);

    if (result == 0) {
        param_1[0xe6] = 1;
        return -1;
    }
    if (result == -1) {
        return -2;
    }

    int *lpAddend = param_1 + 0xdf;
    int *last_field = param_1 + 0xe2;

    param_1[(*lpAddend % 0x18) * 4 + 0x79] = param_1[9];
    param_1[(*lpAddend % 0x18 + 0x1f) * 4] = param_3;
    param_1[(*lpAddend % 0x18) * 4 + 0x7b] = *last_field;
    param_1[9] = param_1[9] + param_2;

    InterlockedIncrementFn increment = *reinterpret_cast<InterlockedIncrementFn *>(g_006690fc);
    increment(reinterpret_cast<long *>(lpAddend));
    increment(reinterpret_cast<long *>(param_1 + 0xe0));
    increment(reinterpret_cast<long *>(last_field));

    return 1;
}
