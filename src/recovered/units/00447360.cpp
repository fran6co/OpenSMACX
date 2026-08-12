// ORIGINAL: 0x00447360 FILE
// name      ?general@MAmbience@@QAEXXZ
// size      266 bytes
// spans     0x00447360-0x0044746A
// prototype void (__thiscall ?general@MAmbience@@QAEXXZ)(MAmbience* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044737E 0x0044738B 0x00447399 0x004473E2 0x004473FA 0x00447461
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00447360
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00447360/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?general@MAmbience@@QAEXXZ  at 0x00447360  (266 bytes)
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

class MAmbience;

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 67, 69, 70
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
    virtual void slot067();  // <-- used
    virtual void slot068();
    virtual void slot069();  // <-- used
    virtual void slot070();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00683bf8 = (int *)0x00683BF8;
static int *const g_00683bfc = (int *)0x00683BFC;
static int *const g_0074c5e8 = (int *)0x0074C5E8;
static int *const g_0074d8ec = (int *)0x0074D8EC;
static int *const g_0074daac = (int *)0x0074DAAC;
static int *const g_0074dab0 = (int *)0x0074DAB0;

class MAmbience { public:
    uint8_t base_storage_[0x58];
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint8_t field_6C_;
    uint8_t field_6D_;
    uint8_t pad_6E_[2];
    uint32_t field_70_;

    void general();
};

// timeGetTime is imported and called through the loaded IAT pointer, twice;
// the emitter's plain global for the slot covers a load of the pointer, and
// this function's own two calls share one loaded value like the original.
typedef unsigned long (__stdcall *TimeGetTimeFn)();

// Slots 67, 69, 70 are called with arguments; the emitter's VCall shim
// declares every slot nullary, so a second shim carries the real arities.
class VCall2 { public:
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
    virtual void slot067(int);  // <-- used
    virtual void slot068();
    virtual void slot069(int, int);  // <-- used
    virtual char slot070(int);  // <-- used, tested via AL
};

void MAmbience::general() {
    TimeGetTimeFn get_time = reinterpret_cast<TimeGetTimeFn>(*g_00669368);
    if ((*reinterpret_cast<unsigned char *>(g_0074c5e8) & 1) == 0) {
        *reinterpret_cast<unsigned char *>(g_0074c5e8) |= 1;
        *g_0074d8ec = get_time();
    }
    VCall2 *target = reinterpret_cast<VCall2 *>(this);
    if (target->slot070(100) == 0) {
        unsigned int now = get_time();
        unsigned int elapsed = now - *reinterpret_cast<unsigned int *>(g_0074d8ec);
        int counter = *g_0074daac + 1;
        *g_0074daac = counter;
        if (elapsed >= 0x1388 && counter >= 5) {
            *g_0074daac = 0;
            target->slot069(*g_00683bf8, *g_0074dab0);
            int old_b = (*g_00683bf8)++;
            target->slot067(old_b);
            if (*g_00683bf8 > 7) {
                *g_00683bf8 = 5;
            }
            int a_val = *g_0074dab0 + 0x96;
            *g_0074dab0 = a_val;
            if (a_val > 0x258) {
                *g_0074dab0 = -600;
            }
            *g_0074d8ec = now;
            int f70 = field_70_ + 0x64;
            field_70_ = f70;
            if (f70 > 0x1f4) {
                field_70_ = -700;
            }
            target->slot069(*g_00683bfc, field_70_);
        }
    }
}
