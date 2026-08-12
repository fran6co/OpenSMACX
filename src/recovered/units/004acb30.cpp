// ORIGINAL: 0x004ACB30 FILE
// name      ?report_security@@YAXXZ
// size      76 bytes
// spans     0x004ACB30-0x004ACB7C
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004A3950
// indirect  0x004ACB60
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004ACB30
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004acb30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?report_security@@YAXXZ  at 0x004ACB30  (76 bytes)
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

class ReportIf;

// ---- callees, declared and never defined (a definition would be inlined) ----
class ReportIf { public:
    void report(int, int, int);
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
// Passed to the call only as an address, never dereferenced here - the
// extern spelling keeps the literal from folding away under /O2.
extern char g_005398e0[];
extern char g_007ae820[];
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_00885f38 = (int *)0x00885F38;
static int *const g_00885f40 = (int *)0x00885F40;
static int *const g_008a4160 = (int *)0x008A4160;

// Vtable shim for the singleton at 0x876478: slot 0xe4/4 = 57.
class VCall { public:
    virtual void slot000(); virtual void slot001(); virtual void slot002();
    virtual void slot003(); virtual void slot004(); virtual void slot005();
    virtual void slot006(); virtual void slot007(); virtual void slot008();
    virtual void slot009(); virtual void slot010(); virtual void slot011();
    virtual void slot012(); virtual void slot013(); virtual void slot014();
    virtual void slot015(); virtual void slot016(); virtual void slot017();
    virtual void slot018(); virtual void slot019(); virtual void slot020();
    virtual void slot021(); virtual void slot022(); virtual void slot023();
    virtual void slot024(); virtual void slot025(); virtual void slot026();
    virtual void slot027(); virtual void slot028(); virtual void slot029();
    virtual void slot030(); virtual void slot031(); virtual void slot032();
    virtual void slot033(); virtual void slot034(); virtual void slot035();
    virtual void slot036(); virtual void slot037(); virtual void slot038();
    virtual void slot039(); virtual void slot040(); virtual void slot041();
    virtual void slot042(); virtual void slot043(); virtual void slot044();
    virtual void slot045(); virtual void slot046(); virtual void slot047();
    virtual void slot048(); virtual void slot049(); virtual void slot050();
    virtual void slot051(); virtual void slot052(); virtual void slot053();
    virtual void slot054(); virtual void slot055(); virtual void slot056();
    virtual void slot057(int, void *, void *);
};
static VCall *const g_00876478 = (VCall *)0x00876478;

void __cdecl report_security() {
    // MNEMONIC_ONLY, one register short of exact: this compile reuses eax
    // (just freed after `idx`'s last use in the cmp) for the vtable-pointer
    // load below, giving the eax-specific 5-byte `mov` form; the original
    // allocates edx there instead, needing the general 6-byte ModRM form.
    // Everything else - instruction order, sizes and the two relocated
    // address-arguments - lines up exactly once the index load is moved
    // ahead of the store (see below); this last register choice did not
    // move under any rephrasing tried.
    int idx = *g_007d392c;
    *g_00885f40 = 7;
    if (g_007ae778[idx] != 5) {
        g_00876478->slot057(0, g_005398e0, g_007ae820);
        return;
    }
    reinterpret_cast<ReportIf *>(g_00885f38)->report(7, *g_008a4160, 0);
}

