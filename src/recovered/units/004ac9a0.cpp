// ORIGINAL: 0x004AC9A0 FILE
// name      ?report_labs@@YAXXZ
// size      76 bytes
// spans     0x004AC9A0-0x004AC9EC
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004A3950
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004AC9A0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004ac9a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?report_labs@@YAXXZ  at 0x004AC9A0  (76 bytes)
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
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_00876478 = (int *)0x00876478;
static int *const g_00885f38 = (int *)0x00885F38;
static int *const g_00885f40 = (int *)0x00885F40;
static int *const g_008a4160 = (int *)0x008A4160;

// Vtable shim (the emitter's own slot detector missed this dispatch: the
// receiver is built from two fixed globals - deref one for the vtable,
// the address of the other for `this` - rather than from an incoming
// `this` parameter, which is what its static scan looks for). Slot
// 0xe4/4 = 57.
class VCallX { public:
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
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057(int, int, int);  // <-- used
};

void __cdecl report_labs() {
    // MNEMONIC_ONLY, closest reached. All 18 mnemonics and every constant
    // agree; the only divergence is register choice in the two mutually
    // exclusive branches - original uses edx for the vtable pointer and
    // eax for the report path's temp, this rebuild uses eax and ecx
    // respectively. `int idx = *g_007d392c;` was needed to get the load
    // of the index to schedule BEFORE the store to g_00885f40 (matching
    // the original's instruction order); that same local appears to move
    // the allocator's later choices for the two independent branches.
    // Tried: no local (wrong instruction order), `register int idx`
    // (identical codegen to plain int - ignored under /O2), and mirroring
    // the report branch with its own named local (no effect). All landed
    // on the same register assignment, which reads as a genuine allocator
    // artifact rather than something a further source respelling fixes.
    int idx = *g_007d392c;
    *g_00885f40 = 2;
    if (g_007ae778[idx] != 5) {
        reinterpret_cast<VCallX *>(g_00876478)->slot057(
            0, reinterpret_cast<int>(g_005398e0), reinterpret_cast<int>(g_007ae820));
        return;
    }
    reinterpret_cast<ReportIf *>(g_00885f38)->report(2, *g_008a4160, 0);
}
