// ORIGINAL: 0x0062D160 sub_62d160 0x0062D160-0x0062D191 FILE
// size      49 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0062D188
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0062D160
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0062d160/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_62d160  at 0x0062D160  (49 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 17
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_009b3ab0 = (int *)0x009B3AB0;
static int *const g_009bc48c = (int *)0x009BC48C;
static int *const g_009bc498 = (int *)0x009BC498;

// The vtable entry is a plain callback taking the object itself and an
// output pointer as explicit stdcall args - the object pointer is pushed
// on the stack rather than passed implicitly in ecx, so a C++ virtual
// call (which never pushes `this`) cannot reproduce it; a function
// pointer of the observed signature does.
typedef int(__stdcall *Handler17)(int, int *);

extern "C" int __cdecl sub_62d160() {
    // Register allocation is swapped from the original throughout: the
    // original keeps the counter in ecx (mov ecx,[...]; the general
    // ModRM form) and the flag in eax (the short A1-encoded
    // mov-eax-from-memory form); this compile puts the counter in eax
    // and the flag in ecx - the opposite pairing. Every attempt at
    // reordering the three loads/declarations (counter-first, flag-first,
    // obj-first, a named counter local, `register`, and nested ifs in
    // place of `&&`) reproduced this same swap, never the original's
    // pairing, so it is recorded as a register-allocator choice rather
    // than a fixable source-form difference. Same instruction count (15),
    // same control flow and same call site; only the eax/ecx roles for
    // the counter and the flag differ, which costs 2 bytes (47 vs 49)
    // because MSVC's A1 short encoding for eax-with-memory is one byte
    // shorter per access than the general ModRM form ecx needs.
    int flag = *g_009bc48c;
    ++*g_009b3ab0;
    int obj = *g_009bc498;
    if (obj != 0 && flag == 0) {
        Handler17 fn = reinterpret_cast<Handler17 *>(*reinterpret_cast<int **>(obj))[17];
        fn(obj, g_009bc48c);
    }
    return *g_009bc48c;
}

