// ORIGINAL: 0x006457C2 FILE
// working copy - scaffold materialised by --work
// name      ??_L@YGXPAXIHP6EX0@Z1@Z
// size      130 bytes
// spans     0x006457C2-0x00645844
// prototype void (__stdcall ??_L@YGXPAXIHP6EX0@Z1@Z)(void*, unsigned int size, int count, void (__thiscall *)(void*), void (__thiscall *)(void*))
// callers   132   call targets   2
// kind      library
// flags     lib;frame;hidden;sp_ready;purged_ok
// calls     0x00645764 0x006457C2
// indirect  0x006457FD

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??_L@YGXPAXIHP6EX0@Z1@Z  at 0x006457C2  (130 bytes)
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
void __stdcall __ArrayUnwind(void*, unsigned int, int, void (*)(void*));

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00646df8 = (int *)0x00646DF8;
static int *const g_00671078 = (int *)0x00671078;

extern "C" int __cdecl _abnormal_termination(void);

// VC6 refuses the `__thiscall` keyword outright ("reserved for future use")
// so a thiscall function pointer has to be smuggled in as a non-virtual
// member-function pointer of an otherwise-opaque class, then reinterpreted
// from the plain code pointer the parameter actually carries.
class NCCtorObj006457c2 { public: void ctor(); };
typedef void (NCCtorObj006457c2::*PMFN006457c2)();

void __stdcall fn_006457c2(void* a1, unsigned int a2, int a3, void (*a4)(void*), void (*a5)(void*)) {
    int i;
    __try {
        for (i = 0; i < a3; i++) {
            (reinterpret_cast<NCCtorObj006457c2*>(a1)->**reinterpret_cast<PMFN006457c2*>(&a4))();
            a1 = (char*)a1 + a2;
        }
    }
    __finally {
        if (_abnormal_termination()) {
            __ArrayUnwind(a1, a2, i, a5);
        }
    }
}
