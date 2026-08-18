// ORIGINAL: 0x006456E4 ??_M@YGXPAXIHP6EX0@Z@Z 0x006456E4-0x00645764 FILE
// RULED-OUT: explicit __thiscall keyword on a free-function-pointer typedef (C4234 reserved keyword); use a member-function-pointer/union shim instead. MISMATCH #12 'and' vs 'mov' remains open.
// working copy - scaffold materialised by --work
// size      128 bytes
// prototype void (__stdcall ??_M@YGXPAXIHP6EX0@Z@Z)(void*, unsigned int size, int count, void (__thiscall *)(void*))
// callers   160   call targets   2
// kind      library
// flags     lib;frame;sp_ready;purged_ok
// calls     0x006456E4 0x00645764
// indirect  0x00645726

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??_M@YGXPAXIHP6EX0@Z@Z  at 0x006456E4  (128 bytes)
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
static int *const g_00671058 = (int *)0x00671058;
class DtorTarget { public: void dtor(); };
typedef void (DtorTarget::*DtorFn)();

union DtorCastU {
    void (*raw)(void*);
    DtorFn member;
};

void __stdcall fn_006456e4(void* a1, unsigned int a2, int a3, void (*a4)(void*)) {
    char *p = (char *)a1 + a2 * a3;
    DtorCastU u;
    u.raw = a4;
    __try {
        for (; a3 > 0; a3--) {
            p -= a2;
            (reinterpret_cast<DtorTarget*>(p)->*u.member)();
        }
    }
    __except(1) {
        __ArrayUnwind(p, a2, a3, a4);
    }
}
