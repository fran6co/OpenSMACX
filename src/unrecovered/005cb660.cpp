// ORIGINAL: 0x005CB660 FILE
// RULED-OUT: nothing beyond the direct InterlockedExchange/Sleep
//            transcription; diverges at #12 (test vs mov)
// working copy - scaffold materialised by --work
// name      sub_5cb660
// size      95 bytes
// spans     0x005CB660-0x005CB6BF
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CB67D 0x005CB696 0x005CB6B1

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cb660  at 0x005CB660  (95 bytes)
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
static int *const g_00669100 = (int *)0x00669100;
static int *const g_0066912c = (int *)0x0066912C;
typedef long (__stdcall *InterlockedExchangeProc)(long *, long);
typedef void (__stdcall *SleepProc)(unsigned int);

extern "C" int __cdecl sub_5cb660(int *param1) {
    int owner = *param1;
    int savedField = 0;
    if (owner != 0) {
        savedField = *reinterpret_cast<int *>(owner + 0x3d0);
    }
    long *target = reinterpret_cast<long *>(reinterpret_cast<char *>(param1) + 0x98);
    (*reinterpret_cast<InterlockedExchangeProc *>(g_00669100))(target, 4);
    int value = static_cast<int>(*target);
    if (savedField == 0) {
        while (value != 5) {
            (*reinterpret_cast<SleepProc *>(g_0066912c))(10);
            value = static_cast<int>(*target);
        }
    } else if (value != 6) {
        do {
            (*reinterpret_cast<SleepProc *>(g_0066912c))(10);
        } while (*target != 6);
        return 0;
    }
    return 0;
}
