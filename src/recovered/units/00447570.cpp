// ORIGINAL: 0x00447570 ?general@UAmbience@@QAEXXZ 0x00447570-0x00447684 FILE
// size      276 bytes
// prototype void (__thiscall ?general@UAmbience@@QAEXXZ)(UAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004475D1 0x004475EB 0x00447620 0x00447650 0x00447675
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447570
// measured tier  MISMATCH
// divergence     15
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447570/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?general@UAmbience@@QAEXXZ  at 0x00447570  (276 bytes)
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

class UAmbience;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 67, 69
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
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067(int);  // <-- used, one int arg (single push before the call)
    virtual void slot068();
    virtual void slot069(int, int);  // <-- used, two int args (two pushes before the call)
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00683c00 = (int *)0x00683C00;
static int *const g_00683c04 = (int *)0x00683C04;
static int *const g_00683c08 = (int *)0x00683C08;
static int *const g_00683c0c = (int *)0x00683C0C;
static int *const g_0074dab4 = (int *)0x0074DAB4;
static int *const g_0074dab8 = (int *)0x0074DAB8;
static int *const g_0074dabc = (int *)0x0074DABC;
static int *const g_0074dac0 = (int *)0x0074DAC0;
static int *const g_0074dac4 = (int *)0x0074DAC4;

class UAmbience { public:
    uint8_t pad_0_[0x6C];
    uint32_t field_6c_;

    void general();
};

void UAmbience::general() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<uint8_t *>(self + 0x6C) != 0) {
        return;
    }

    int dac4 = *g_0074dac4;
    int dab4 = *g_0074dab4;
    int dab8 = *g_0074dab8;
    dac4++;
    dab4++;
    dab8++;
    *g_0074dab4 = dab4;
    int dac0 = *g_0074dac0;
    dac0++;
    bool rollA = (dac4 == 0xF);
    *g_0074dac4 = dac4;
    *g_0074dab8 = dab8;
    *g_0074dac0 = dac0;

    if (rollA) {
        *g_00683c0c = (*g_00683c0c == 4) + 4;
        reinterpret_cast<VCall *>(this)->slot067(*g_00683c0c);
        *g_0074dac4 = 0;
    }
    if (*g_0074dab4 == 9) {
        reinterpret_cast<VCall *>(this)->slot067(0);
        *g_0074dab4 = 0;
    }
    if (*g_0074dab8 == 6) {
        *g_00683c00 = (*g_00683c00 != 2) + 1;
        reinterpret_cast<VCall *>(this)->slot069(*g_00683c00, *g_0074dabc);
        *g_0074dabc += 0x64;
        if (*g_00683c00 > 0x258) {
            *g_00683c00 = -0x258;
        }
        reinterpret_cast<VCall *>(this)->slot067(*g_00683c00);
        *g_0074dab8 = 0;
    }
    if (*g_0074dac0 == *g_00683c08) {
        reinterpret_cast<VCall *>(this)->slot067(*g_00683c04);
        *g_0074dac0 = 0;
    }
}
