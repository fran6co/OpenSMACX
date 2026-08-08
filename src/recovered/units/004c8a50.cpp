// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004C8A50
// name           ??1VoiceRx@@QAE@XZ
// size           262 bytes
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004c8a50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1VoiceRx@@QAE@XZ  at 0x004C8A50  (262 bytes)
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

class VoiceRx;

// ---- callees, declared and never defined (a definition would be inlined) ----
int __cdecl fn_0064557f();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00659fb4 = (int *)0x00659FB4;
static int *const g_0066e3c0 = (int *)0x0066E3C0;
static int *const g_0066e444 = (int *)0x0066E444;
static int *const g_0066e82c = (int *)0x0066E82C;
static int *const g_00679270 = (int *)0x00679270;
static int *const g_0090db1c = (int *)0x0090DB1C;
static int *const g_0090db20 = (int *)0x0090DB20;
static int *const g_0090db28 = (int *)0x0090DB28;
static int *const g_0090db7c = (int *)0x0090DB7C;

class VoiceRx { public:
    ~VoiceRx();
};

// The provided VCall shim's slot002 is nullary; this call needs an int
// argument (`push 1; call [eax+8]`), so a second, differently-shaped shim
// is used instead of redeclaring the already-complete VCall class.
class VCall2 { public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2(int);
};

typedef void(__cdecl *FreeProc)(void *);

VoiceRx::~VoiceRx() {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self) = 0x66e82c;

    void *p = *reinterpret_cast<void **>(self + 0x3c);
    if (p != 0) {
        reinterpret_cast<VCall2 *>(p)->slot2(1);
        *reinterpret_cast<void **>(self + 0x3c) = 0;
    }

    p = *reinterpret_cast<void **>(self + 0x4c);
    if (p != 0) {
        ::operator delete(p);
        *reinterpret_cast<void **>(self + 0x4c) = 0;
    }

    *reinterpret_cast<int *>(self) = 0x66e3c0;

    p = *reinterpret_cast<void **>(self + 0x4c);
    if (p != 0) {
        ::operator delete(p);
        *reinterpret_cast<void **>(self + 0x4c) = 0;
    }

    p = *reinterpret_cast<void **>(self + 0x3c);
    if (p != 0) {
        if (*g_0090db7c == 0) {
            (*reinterpret_cast<FreeProc *>(g_0090db28))(p);
        }
        *reinterpret_cast<void **>(self + 0x3c) = 0;
    }

    if (*reinterpret_cast<unsigned char *>(self + 0x40) & 2) {
        void *prev = *reinterpret_cast<void **>(self + 0x44);
        void *next = *reinterpret_cast<void **>(self + 0x48);
        if (prev != 0) {
            *reinterpret_cast<void **>(reinterpret_cast<char *>(prev) + 0x48) = next;
        } else {
            *reinterpret_cast<void **>(g_0090db20) = next;
        }
        if (next != 0) {
            *reinterpret_cast<void **>(reinterpret_cast<char *>(next) + 0x44) = prev;
        } else {
            *reinterpret_cast<void **>(g_0090db1c) = prev;
        }
        *reinterpret_cast<void **>(self + 0x48) = 0;
        *reinterpret_cast<void **>(self + 0x44) = 0;
        *reinterpret_cast<unsigned char *>(self + 0x40) &= ~2;
    }

    *reinterpret_cast<int *>(self) = 0x66e444;
}
