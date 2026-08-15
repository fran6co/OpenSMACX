// ORIGINAL: 0x005CE2F0 BYTE_EXACT FILE
// name      sub_5ce2f0
// size      70 bytes
// spans     0x005CE2F0-0x005CE336
// prototype 
// callers   2   call targets   0
// kind      
// flags     
// calls     (none)
// indirect  0x005CE30B 0x005CE310 0x005CE31F 0x005CE324
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ce2f0  at 0x005CE2F0  (70 bytes)
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

// SIGNATURE CHANGE: `[ecx+0x3b0]`/`[ecx+0x3b4]` reads with no stack access
// mean this is a __thiscall member, not the free `sub_5ce2f0()` the contract
// gave - see PROPOSALS in the agent report. No name for the real class is
// known, so this introduces a minimal placeholder receiver.
class SyncObject { public:
    int release();
};
int SyncObject::release() {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x3b4) != 0) {
        long *target = reinterpret_cast<long *>(self + 0x3b0);
        typedef long (__stdcall *PFN_InterlockedExchange)(long *, long);
        PFN_InterlockedExchange interlockedExchange = (PFN_InterlockedExchange)*g_00669100;
        interlockedExchange(target, 0);
        long result = interlockedExchange(target, 0);
        if (result == 0) {
            typedef void (__stdcall *PFN_Sleep)(unsigned int);
            PFN_Sleep sleep = (PFN_Sleep)*g_0066912c;
            do {
                sleep(10);
                result = interlockedExchange(target, 0);
            } while (result == 0);
        }
        return 0;
    }
    return 1;
}
