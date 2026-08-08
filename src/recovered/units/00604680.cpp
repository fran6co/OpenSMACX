// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00604680
// name           ?close_class@BasePop@@QAAXXZ
// size           109 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00604680/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close_class@BasePop@@QAAXXZ  at 0x00604680  (109 bytes)
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

class BasePop;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" void free(void *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b8d80 = (int *)0x009B8D80;
static int *const g_009b8d84 = (int *)0x009B8D84;
static int *const g_009b8d98 = (int *)0x009B8D98;
static int *const g_009b8da8 = (int *)0x009B8DA8;
static int *const g_009bc074 = (int *)0x009BC074;
static int *const g_009bc078 = (int *)0x009BC078;

class BasePop { public:
    void __cdecl close_class();
};

// The provided VCall shim's slot000 is nullary; this call needs an int
// argument (`push 1; call [eax]`), so a second, differently-shaped shim is
// used instead of redeclaring the already-complete VCall class.
class VCall1 { public:
    virtual void slot0(int);
};

void __cdecl BasePop::close_class() {
    int p1 = *g_009b8d80;
    *g_009b8d98 = 0;
    *g_009b8da8 = 0;

    if (p1 != 0) {
        free(reinterpret_cast<void *>(p1));
        *g_009b8d80 = 0;
    }
    if (*g_009b8d84 != 0) {
        free(reinterpret_cast<void *>(*g_009b8d84));
        *g_009b8d84 = 0;
    }
    if (*g_009bc074 != 0) {
        reinterpret_cast<VCall1 *>(*g_009bc074)->slot0(1);
        *g_009bc074 = 0;
    }
    if (*g_009bc078 != 0) {
        reinterpret_cast<VCall1 *>(*g_009bc078)->slot0(1);
        *g_009bc078 = 0;
    }
}
