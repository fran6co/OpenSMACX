// ORIGINAL: 0x00428DF0 ?show_credits@@YAXXZ 0x00428DF0-0x00428E5B;0x0065315A-0x0065316F FILE
// working copy - scaffold materialised by --work
// size      128 bytes
// prototype 
// callers   4   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00428780 0x004289F0 0x0064557F 0x0064558A
// indirect  0x00428E4A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?show_credits@@YAXXZ  at 0x00428DF0  (128 bytes)
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

class Credits;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Credits { public:
    Credits();
    void exec();
};

int __cdecl fn_0064557f();
int __cdecl fn_0064558a();

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
static int *const g_00653165 = (int *)0x00653165;
static int *const g_00673ab8 = (int *)0x00673AB8;
// `new Credits()` alone uses sizeof(Credits), which is 1 for this
// deliberately opaque (fieldless) class - wrong against the disassembly's
// explicit `push 0x2d6c` operator-new size. Call operator new with the
// real size directly and placement-construct on the result; a local
// placement-delete pairing is declared purely so the compiler keeps the
// EH unwind path (it drops the SEH frame entirely without one).
inline void *operator new(unsigned int, void *p) { return p; }
static inline void operator delete(void *, void *) {}

// The vtable call passes an explicit flag (`push 1`) - a scalar deleting
// destructor slot, not a plain virtual call.
class DtorVCall { public:
    virtual void slot000(int);
};

void __cdecl show_credits() {
    void *pv = operator new(0x2d6cU);
    Credits *credits;
    if (pv == 0) {
        credits = 0;
    } else {
        credits = new (pv) Credits();
    }
    credits->exec();
    if (credits != 0) {
        reinterpret_cast<DtorVCall *>(credits)->slot000(1);
    }
}
